#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sockfd, choice;
    char domain[1024], buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    // 1. Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost

    printf("UDP DNS Client initialized.\n");

    while (1) {
        printf("\n=== Client Menu ===\n");
        printf("1. Look up Domain Name\n");
        printf("2. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            break; // Exit on non-numeric input
        }
        getchar(); // consume newline

        if (choice == 2) {
            break;
        } else if (choice == 1) {
            // Enter website name
            printf("Enter domain name (e.g., google.com): ");
            fgets(domain, sizeof(domain), stdin);
            domain[strcspn(domain, "\r\n")] = '\0'; // Strip newline

            if (strlen(domain) == 0) continue;

            // 2. Send query to DNS server
            sendto(sockfd, domain, strlen(domain), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

            // 3. Receive response (IP address) from DNS server
            // Added a simple buffer clean before receiving
            memset(buffer, 0, sizeof(buffer));
            int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&serverAddr, &addrLen);

            if (n < 0) {
                perror("Receive failed");
                continue;
            }

            buffer[n] = '\0'; // Null terminate

            // Handle potential empty response visually
            if (strlen(buffer) == 0) {
                 printf("Resolved IP Address: <Server returned empty response>\n");
            } else {
                 printf("Resolved IP Address: %s\n", buffer);
            }
        } else {
            printf("Invalid choice. Please enter 1 or 2.\n");
        }
    }

    close(sockfd);
    printf("Client exited.\n");
    return 0;
}
