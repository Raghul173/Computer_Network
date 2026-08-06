// This C program simulates network protocol encapsulation across the Transport, Network, and Data Link layers, culminating in 
a demonstration of PPP (Point-to-Point Protocol) framing that includes byte stuffing for transparency and a 16-bit checksum for error detection.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define F_SZ 8

#define PPP_FLAG 0x7E
#define PPP_ESC  0x7D
#define PPP_ADDR 0xFF
#define PPP_CTRL 0x03

struct Node {
    char url[50], ip[20], mac[20];
    int port;
    struct Node *next;
};

struct Node *table[SIZE] = {NULL};

char srcURL[50] = "Default Source";
char srcIP[20] = "192.168.1.10";
char srcMAC[20] = "11:22:33:44:55:66";
int srcPort = 51309;

// 1. Hash function
int hash(const char url[]) {
    int sum = 0;
    for(int i = 0; url[i]; i++) sum += url[i];
    return sum % SIZE;
}

// 2. Insert function
void insert(const char url[], const char ip[], const char mac[], int port) {
    int idx = hash(url);
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->url, url);
    strcpy(newNode->ip, ip);
    strcpy(newNode->mac, mac);
    newNode->port = port;
    newNode->next = table[idx];
    table[idx] = newNode;
}

// 3. Search function
struct Node* search(const char url[]) {
    struct Node *tmp = table[hash(url)];
    while(tmp) {
        if(strcmp(tmp->url, url) == 0) return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

// 4. Delete function
// Renamed delete -> deleteURL because "delete" is a reserved keyword in
// C++ (it's the operator that frees memory allocated with "new"), so
// g++ refuses to let it be used as a function name.
void deleteURL(const char url[]) {
    int idx = hash(url);
    struct Node *tmp = table[idx];
    struct Node *prev = NULL;

    while (tmp != NULL && strcmp(tmp->url, url) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }

    if (tmp == NULL) return;

    if (prev == NULL) {
        table[idx] = tmp->next;
    } else {
        prev->next = tmp->next;
    }
    free(tmp);
}

// 5. Print URL Table
void URLTable() {
    printf("\n================ URL TABLE ================\n");
    printf("%-20s %-18s %-19s %-5s\n", "URL", "IP", "MAC", "PORT");
    for(int i = 0; i < SIZE; i++) {
        struct Node *tmp = table[i];
        while(tmp) {
            printf("%-20s %-18s %-19s %-5d\n", tmp->url, tmp->ip, tmp->mac, tmp->port);
            tmp = tmp->next;
        }
    }
    printf("===========================================\n");
}

// 6. Preload default entries
void preload() {
    insert("www.mail.com", "142.250.183.14", "AA:BB:CC:DD:EE:01", 25);
    insert("www.whatsapp.com", "142.250.190.46", "AA:BB:CC:DD:EE:02", 443);
    insert("www.facebook.com", "157.240.22.35", "AA:BB:CC:DD:EE:03", 80);
    insert("www.google.com", "142.250.190.47", "AA:BB:CC:DD:EE:04", 443);
}

// 7. Print byte in binary
void printByte(unsigned char n) {
    for (int i = 7; i >= 0; i--) printf("%d", (n >> i) & 1);
}

// 8. Print port in binary
void printPort(int port) {
    for (int i = 15; i >= 0; i--) printf("%d", (port >> i) & 1);
}

// 9. Print IP in binary
void printIP(const char ip[]) {
    int a, b, c, d;
    if (sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        printByte(a); printByte(b); printByte(c); printByte(d);
    }
}

// 10. Print MAC in binary
void printMAC(const char mac[]) {
    unsigned int x[6];
    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &x[0], &x[1], &x[2], &x[3], &x[4], &x[5]) == 6) {
        for (int i = 0; i < 6; i++) printByte(x[i]);
    }
}

// 11. Compute 16-bit one's-complement checksum over byte buffer
unsigned short computeChecksum(unsigned char *data, int length) {
    unsigned int sum = 0;
    for (int i = 0; i < length; i += 2) {
        if (i + 1 < length)
            sum += (data[i] << 8) | data[i + 1];
        else
            sum += (data[i] << 8);
    }
    while (sum > 0xFFFF) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (unsigned short)(~sum);
}

// 12. Show OSI layers
void showLayers(char msg[], int len, struct Node *dest) {
    printf("\nMessage :\n");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printf("\n\n");

    printf("========= TRANSPORT LAYER =========\n");
    printf("Source Port      : "); printPort(srcPort); printf("\n");
    printf("Destination Port : "); printPort(dest->port); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32);

    printf("========= Network Layer =========\n\n");
    printf("Source IP      : "); printIP(srcIP); printf("\n");
    printf("Destination IP : "); printIP(dest->ip); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    printIP(srcIP); printf(" "); printIP(dest->ip);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64);

    printf("========= Data Link Layer =========\n");
    printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
    printf("Destination MAC: "); printMAC(dest->mac); printf("\n");
    printf("Stream           : ");
    for(int i = 0; i < len; i++) { printByte(msg[i]); printf(" "); }
    printPort(srcPort); printf(" "); printPort(dest->port); printf(" ");
    printIP(srcIP); printf(" "); printIP(dest->ip); printf(" ");
    printMAC(srcMAC); printf(" "); printMAC(dest->mac);
    printf("\nTotal Bits       : %d bits\n\n", (len * 8) + 32 + 64 + 96);
}

// 13. PPP Sender function
void pppSender(unsigned char *data, int len) {
    FILE *fp = fopen("transmitted_ppp.txt", "w");
    if (!fp) return;

    printf("\n========= SENDER INTERMEDIATE PROCESSING (PPP) =========\n");

    // 1. Start Flag
    fprintf(fp, "%02X ", PPP_FLAG);
    printf("-> Appending Start Flag      : "); printByte(PPP_FLAG); printf("\n");

    // 2. Address & Control Fields
    fprintf(fp, "%02X %02X ", PPP_ADDR, PPP_CTRL);
    printf("-> Appending Address Block   : "); printByte(PPP_ADDR); printf("\n");
    printf("-> Appending Control Block   : "); printByte(PPP_CTRL); printf("\n");

    // 3. Protocol field (0x00 0x21)
    fprintf(fp, "00 21 ");
    printf("-> Appending Protocol Ident. : "); printByte(0x00); printf(" "); printByte(0x21); printf("\n");

    /* Display Original Payload */
    printf("\nOriginal Payload (Binary):\n");
    for (int i = 0; i < len; i++) {
        printByte(data[i]); printf(" ");
    }
    printf("\n");

    /* Display Complete Stuffed Frame Header info */
    printf("\n Stuffed Frame\n");
    printByte(PPP_FLAG); printf(" "); printByte(PPP_ADDR); printf(" ");
    printByte(PPP_CTRL); printf(" "); printByte(0x00); printf(" "); printByte(0x21); printf(" ");

    // 4. Stuffed Payload Data
    printf("\n-> Processing and Byte-Stuffing Payload Stream:\n   Final Frame Sequence: [ ");

    for (int i = 0; i < len; i++) {
        if (data[i] == PPP_FLAG || data[i] == PPP_ESC) {
            fprintf(fp, "%02X ", PPP_ESC);
            fprintf(fp, "%02X ", data[i]);

            printByte(PPP_ESC); printf(" "); printByte(data[i]); printf(" ");
        } else {
            fprintf(fp, "%02X ", data[i]);

            printByte(data[i]); printf(" ");
        }
    }
    printf("]\n");

    // 5. 16-bit Checksum
    unsigned short chk = computeChecksum(data, len);
    unsigned char chkHigh = (chk >> 8) & 0xFF;
    unsigned char chkLow = chk & 0xFF;

    fprintf(fp, "%02X %02X ", chkHigh, chkLow);
    printf("-> Appending Checksum Field  : "); printByte(chkHigh); printf(" "); printByte(chkLow); printf("\n");

    // 6. End Flag
    fprintf(fp, "%02X\n", PPP_FLAG);
    printf("-> Appending End Flag        : "); printByte(PPP_FLAG); printf("\n");
    printf("========================================================\n");

    fclose(fp);
    printf("[Sender] Full structured PPP Frame saved to 'transmitted_ppp.txt'\n");
}

// 14. PPP Receiver function
void pppReceiver(const char *inFileName) {
    FILE *fp = fopen(inFileName, "r");
    if (!fp) { printf("Error reading transmission file!\n"); return; }

    printf("\n========= RECEIVER SIDE (PPP BYTE DESTUFFING) =========\n");

    unsigned int bytes[1000];
    int count = 0;
    while (fscanf(fp, "%x", &bytes[count]) == 1 && count < 1000) {
        count++;
    }
    fclose(fp);

    if (count < 8) return;

    printf("Extracted Elements from PPP Frame:\n");
    printf("Start Flag Detected : "); printByte(bytes[0]); printf("\n");
    printf("Address Field       : "); printByte(bytes[1]); printf("\n");
    printf("Control Field       : "); printByte(bytes[2]); printf("\n");
    printf("Protocol Field H    : "); printByte(bytes[3]); printf("\n");
    printf("Protocol Field L    : "); printByte(bytes[4]); printf("\n");

    unsigned char payload[1000];
    int pLen = 0;

    printf("\nReading Payload Data Stream: ");
    for (int i = 5; i < count - 3; i++) {
        if (bytes[i] == PPP_ESC) {
            i++;
        }
        payload[pLen] = (unsigned char)bytes[i];
        printByte(payload[pLen]); printf(" ");
        pLen++;
    }
    payload[pLen] = '\0';
    printf("\n");

    unsigned short recChecksum = (bytes[count - 3] << 8) | bytes[count - 2];
    printf("Checksum Field      : "); printByte(bytes[count - 3]); printf(" "); printByte(bytes[count - 2]); printf("\n");
    printf("End Flag Detected   : "); printByte(bytes[count - 1]); printf("\n");

    unsigned short calcChecksum = computeChecksum(payload, pLen);
    printf("\nComputed Checksum   : "); printByte((calcChecksum >> 8) & 0xFF); printf(" "); printByte(calcChecksum & 0xFF); printf("\n");
    if (calcChecksum == recChecksum) {
        printf("Checksum Status     : CHECKSUM MATCHED (NO ERROR)\n");
    } else {
        printf("Checksum Status     : CHECKSUM ERROR DETECTED!\n");
    }

    printf("Decoded Message     : %s\n", payload);
    printf("=======================================================\n");
}

// 15. Show Frame contents
void showFrames(char msg[], int len, int totalFrames, struct Node *dest) {
    printf("====Frame Contents======\n");
    for(int i = 0; i < totalFrames; i++) {
        printf("\n-----------------------------------------\n");
        int packetNo = (i / 2) + 1;
        printf("Packet No : %d\n", packetNo);
        printf("Frame No  : %d\n", i + 1);
        printf("Source Port      : "); printPort(srcPort); printf("\n");
        printf("Destination Port : "); printPort(dest->port); printf("\n\n");
        printf("Source IP      : "); printIP(srcIP); printf("\n");
        printf("Destination IP : "); printIP(dest->ip); printf("\n");
        printf("Source MAC     : "); printMAC(srcMAC); printf("\n");
        printf("Destination MAC: "); printMAC(dest->mac); printf("\n");

        printf("Frame Data     : ");
        for(int j = 0; j < F_SZ; j++) {
            int cur = (i * F_SZ) + j;
            if(cur < len) { printByte(msg[cur]); printf(" "); }
            else { printByte(0); printf(" "); }
        }
        printf("\nTail           : 00000000\n");
        printf("-----------------------------------------\n");
    }
}

// 16. Main function
int main() {
    char fn[50], url[100], msg[1000] = "";
    FILE *fp;
    int ch, idx = 0, m, cho;

    preload();

    while(1) {
        URLTable();
        printf("\n========= MAIN MENU (PPP PROTOCOL RUNNER) =========\n");
        printf("1. Hash Table Management\n");
        printf("2. Proceed to Data Framing & Run PPP\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &m) != 1) return 0;

        if (m == 1) {
            printf("\n--- Hash Table Functions ---\n");
            printf("1. Add URL Entry\n");
            printf("2. Delete URL Entry\n");
            printf("3. Back\n");
            if(scanf("%d", &cho) != 1) continue;

            if (cho == 1) {
                char newUrl[50], newIp[20], newMac[20]; int newPort;
                printf("Enter URL: "); scanf("%49s", newUrl);
                printf("Enter IP: "); scanf("%19s", newIp);
                printf("Enter MAC: "); scanf("%19s", newMac);
                printf("Enter Port: "); scanf("%d", &newPort);
                insert(newUrl, newIp, newMac, newPort);
            } else if (cho == 2) {
                char delUrl[50];
                printf("Enter URL to delete: "); scanf("%49s", delUrl);
                deleteURL(delUrl);
            }
        } else if (m == 2) {
            break;
        } else {
            return 0;
        }
    }

    printf("\nEnter Source URL from table: ");
    scanf("%99s", url);
    struct Node *srcNode = search(url);
    if(srcNode) {
        strcpy(srcURL, srcNode->url);
        strcpy(srcIP, srcNode->ip);
        strcpy(srcMAC, srcNode->mac);
        srcPort = srcNode->port;
    }

    printf("Enter Destination URL: ");
    scanf("%99s", url);
    struct Node *dest = search(url);
    if(!dest) return 0;

    printf("Enter File Name: ");
    scanf("%49s", fn);

    fp = fopen(fn, "r");
    if(!fp) return 0;

    while((ch = fgetc(fp)) != EOF && idx < 999) msg[idx++] = (char)ch;
    msg[idx] = '\0';
    fclose(fp);

    int len = strlen(msg);
    int totalFrames = len / F_SZ + (len % F_SZ != 0);

    showLayers(msg, len, dest);
    showFrames(msg, len, totalFrames, dest);

    pppSender((unsigned char*)msg, len);
    pppReceiver("transmitted_ppp.txt");

    return 0;
}
