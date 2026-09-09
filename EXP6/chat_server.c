#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sockfd;
    char buffer[1024], message[1024];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen;

    struct sockaddr_in clients[10];
    int clientCount = 0;

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
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 2. Bind socket
    if (bind(sockfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        printf("Bind failed\n");
        close(sockfd);
        return 1;
    }

    printf("UDP Chat Server is running...\n");
    printf("Waiting for clients...\n");

    while (1)
    {
        addrLen = sizeof(clientAddr);

        // 3. Receive message
        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&clientAddr, &addrLen);

        if (n < 0)
        {
            printf("Receive failed\n");
            break;
        }

        buffer[n] = '\0';

        // Find whether this client is already registered
        int clientIndex = -1;

        for (int i = 0; i < clientCount; i++)
        {
            if (clients[i].sin_port == clientAddr.sin_port &&
                clients[i].sin_addr.s_addr == clientAddr.sin_addr.s_addr)
            {
                clientIndex = i;
                break;
            }
        }

        // New client
        if (clientIndex == -1)
        {
            clientIndex = clientCount;
            clients[clientCount] = clientAddr;
            clientCount++;

            printf("Client%d connected.\n", clientIndex + 1);

            // Send assigned client number
            sprintf(message, "Client%d", clientIndex + 1);

            sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr *)&clientAddr, addrLen);

            continue;
        }

        // Client sends exit
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Client%d disconnected.\n",
                   clientIndex + 1);
            continue;
        }

        // Display message
        printf("Client%d: %s\n",
               clientIndex + 1, buffer);

        // Server reply
        printf("Server reply to Client%d: ",
               clientIndex + 1);

        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&clientAddr, addrLen);

        if (strcmp(message, "exit") == 0)
        {
            printf("Chat ended.\n");
            break;
        }
    }

    close(sockfd);

    return 0;
}
