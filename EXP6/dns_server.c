#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define TABLE_SIZE 10
#define DB_FILE "dns_db.txt"
#define MAX_LEN 100

// --- Structures ---

struct HashNode {
    char domain[MAX_LEN];
    char ip[MAX_LEN];
    struct HashNode *next;
};

// --- Global Variables ---
struct HashNode *hashTable[TABLE_SIZE];
int server_sockfd; // Listener socket FD
pid_t child_pid;   // PID of the DNS child process

// --- Hash Table Functions ---

// Simple djb2-like hash function
unsigned int hash(const char *str) {
    unsigned int hashValue = 0;
    while (*str) {
        hashValue = (hashValue * 31) + *str;
        str++;
    }
    return hashValue % TABLE_SIZE;
}

// Free all memory in the hash table
void freeTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        struct HashNode *current = hashTable[i];
        while (current != NULL) {
            struct HashNode *temp = current;
            current = current->next;
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

// Insert new or update existing domain
void insert_or_update(const char *domain, const char *ip) {
    unsigned int slot = hash(domain);
    struct HashNode *current = hashTable[slot];

    // Check if exists, update if found
    while (current != NULL) {
        if (strcmp(current->domain, domain) == 0) {
            strcpy(current->ip, ip);
            return;
        }
        current = current->next;
    }

    // Allocate and insert new node
    struct HashNode *newNode = (struct HashNode *)malloc(sizeof(struct HashNode));
    if (!newNode) {
        perror("Memory allocation failed");
        return;
    }
    strcpy(newNode->domain, domain);
    strcpy(newNode->ip, ip);
    newNode->next = hashTable[slot];
    hashTable[slot] = newNode;
}

// Delete a domain from hash table
int delete_domain(const char *domain) {
    unsigned int slot = hash(domain);
    struct HashNode *current = hashTable[slot];
    struct HashNode *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->domain, domain) == 0) {
            if (prev == NULL) {
                hashTable[slot] = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return 1; // Success
        }
        prev = current;
        current = current->next;
    }
    return 0; // Not found
}

// Lookup IP for a domain
const char* search(const char *domain) {
    unsigned int slot = hash(domain);
    struct HashNode *current = hashTable[slot];
    while (current != NULL) {
        if (strcmp(current->domain, domain) == 0) {
            return current->ip;
        }
        current = current->next;
    }
    return NULL; // Not found
}

// --- Database Persistence (File I/O) ---

// Load data from text file into memory
void loadDatabase() {
    freeTable(); // Clear current memory
    FILE *file = fopen(DB_FILE, "r");

    if (!file) {
        // If file doesn't exist, create it
        file = fopen(DB_FILE, "w");
        if (file) fclose(file);
        return;
    }

    char domain[MAX_LEN], ip[MAX_LEN];
    int count = 0;
    // Format: domain_name ip_address
    while (fscanf(file, "%s %s", domain, ip) != EOF) {
        insert_or_update(domain, ip);
        count++;
    }
    fclose(file);
    // printf("[Debug] Loaded %d records from DB.\n", count); // Optional debug
}

// Save current memory hash table to text file
void saveDatabase() {
    FILE *file = fopen(DB_FILE, "w");
    if (!file) {
        perror("Error opening DB file for saving");
        return;
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        struct HashNode *current = hashTable[i];
        while (current != NULL) {
            fprintf(file, "%s %s\n", current->domain, current->ip);
            current = current->next;
        }
    }
    fclose(file);
}

// Pretty print records to console
void displayDatabase() {
    printf("\n--- Current DNS Records ---\n");
    printf("%-30s | %s\n", "Domain Name", "IP Address");
    printf("-------------------------------------------\n");
    int count = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        struct HashNode *current = hashTable[i];
        while (current != NULL) {
            printf("%-30s | %s\n", current->domain, current->ip);
            current = current->next;
            count++;
        }
    }
    printf("-------------------------------------------\n");
    printf("Total records: %d\n\n", count);
}

// --- Child Process: DNS Listener ---

// Signal handler allows child to refresh data instantly when parent updates file
void sigusr1_handler(int sig) {
    loadDatabase();
    // printf("[Child] Database reloaded via signal notification.\n");
}

void dns_server_process() {
    // 1. Setup Signal Handler for DB reloads
    signal(SIGUSR1, sigusr1_handler);

    // 2. Initial Load
    loadDatabase();

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[1024];

    // 3. Create Socket
    server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sockfd < 0) {
        perror("[Child] Socket creation failed");
        exit(1);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 4. Bind
    if (bind(server_sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[Child] Bind failed");
        close(server_sockfd);
        exit(1);
    }

    printf("[Child] DNS Listener started on port 5000 (PID: %d)\n", getpid());

    // 5. Listen Loop
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int n = recvfrom(server_sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&clientAddr, &addrLen);
        if (n < 0) continue;

        // Clean string input
        buffer[strcspn(buffer, "\r\n")] = 0;
        // printf("[Child] Query for: %s\n", buffer); // Optional debug

        // Lookup
        const char *found_ip = search(buffer);
        char response[1024];

        if (found_ip != NULL) {
            strcpy(response, found_ip);
        } else {
            strcpy(response, "Error: Domain not found");
        }

        // Send Response
        sendto(server_sockfd, response, strlen(response), 0, (struct sockaddr *)&clientAddr, addrLen);
    }
}

// --- Parent Process: Admin Console ---

// Cleanup on exit
void sigint_handler(int sig) {
    printf("\n[Parent] Shutting down server...\n");
    if (child_pid > 0) {
        kill(child_pid, SIGKILL); // Terminate child
        wait(NULL); // Wait for child to exit
    }
    close(server_sockfd);
    printf("[Parent] Server stopped cleanly.\n");
    exit(0);
}

int main() {
    // 1. Handle Ctrl+C
    signal(SIGINT, sigint_handler);

    // Initialize Hash Table memory to NULL
    for(int i=0; i<TABLE_SIZE; i++) hashTable[i] = NULL;

    // 2. Fork processes
    child_pid = fork();

    if (child_pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    if (child_pid == 0) {
        // --- Child Executes ---
        dns_server_process();
    } else {
        // --- Parent Executes ---
        int choice;
        char domain[MAX_LEN], ip[MAX_LEN];

        printf("\n========================================\n");
        printf("   UDP DNS Server & Admin Console\n");
        printf("   Parent PID: %d | Child PID: %d\n", getpid(), child_pid);
        printf("========================================\n");

        while (1) {
            printf("\n1. Display Records\n");
            printf("2. Add/Update Record\n");
            printf("3. Delete Record\n");
            printf("4. Exit and Save\n");
            printf("Enter choice: ");

            if (scanf("%d", &choice) != 1) {
                scanf("%*s"); // Clear bad input
                continue;
            }
            getchar(); // Consume newline

            switch (choice) {
                case 1:
                    // Parent reloads just to be safe before displaying
                    loadDatabase();
                    displayDatabase();
                    break;
                case 2:
                    printf("Enter domain name: ");
                    fgets(domain, MAX_LEN, stdin);
                    domain[strcspn(domain, "\r\n")] = 0;
                    if(strlen(domain) == 0) { printf("Domain cannot be empty.\n"); break;}

                    printf("Enter IP address: ");
                    fgets(ip, MAX_LEN, stdin);
                    ip[strcspn(ip, "\r\n")] = 0;
                    if(strlen(ip) == 0) { printf("IP cannot be empty.\n"); break;}

                    insert_or_update(domain, ip);
                    saveDatabase();

                    // CRITICAL: Signal child to reload file immediately
                    kill(child_pid, SIGUSR1);
                    printf("Record added/updated and DNS process notified.\n");
                    break;
                case 3:
                    printf("Enter domain to delete: ");
                    fgets(domain, MAX_LEN, stdin);
                    domain[strcspn(domain, "\r\n")] = 0;

                    if (delete_domain(domain)) {
                        saveDatabase();
                        // CRITICAL: Signal child to reload file immediately
                        kill(child_pid, SIGUSR1);
                        printf("Record deleted and DNS process notified.\n");
                    } else {
                        printf("Domain not found in database.\n");
                    }
                    break;
                case 4:
                    sigint_handler(SIGINT); // Trigger cleanup
                    break;
                default:
                    printf("Invalid choice.\n");
            }
        }
    }

    return 0;
}
