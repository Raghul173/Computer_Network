#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sockfd;
    char message[1024], buffer[1024];
    char clientName[50];

    struct sockaddr_in serverAddr;
    socklen_t addrLen = sizeof(serverAddr);

    // 1. Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. Contact server
    strcpy(message, "CONNECT");

    sendto(sockfd, message, strlen(message), 0,
           (struct sockaddr *)&serverAddr,
           sizeof(serverAddr));

    // Receive client number from server
    int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&serverAddr,
                     &addrLen);

    buffer[n] = '\0';

    strcpy(clientName, buffer);

    printf("Connected to UDP Chat Server as %s.\n", clientName);
    printf("Type 'exit' to quit.\n");

    while (1)
    {
        // 3. Enter message
        printf("%s: ", clientName);

        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        // Send message
        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr));

        if (strcmp(message, "exit") == 0)
        {
            break;
        }

        // 4. Receive server reply
        n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr *)&serverAddr,
                     &addrLen);

        if (n < 0)
        {
            printf("Receive failed\n");
            break;
        }

        buffer[n] = '\0';

        if (strcmp(buffer, "exit") == 0)
        {
            printf("Server has ended the chat.\n");
            break;
        }

        printf("Server: %s\n", buffer);
    }

    close(sockfd);

    return 0;
}
