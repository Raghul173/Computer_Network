#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int clientCount = 0;

void handle_client(int clientfd, int clientNumber)
{
    char buffer[1024];
    char message[1024];

    /*
     * Send assigned client number
     */
    sprintf(message, "Client%d", clientNumber);

    send(clientfd, message, strlen(message), 0);

    printf("Client%d connected.\n", clientNumber);

    while (1)
    {
        /*
         * Receive message from client
         */
        int n = recv(clientfd,
                     buffer,
                     sizeof(buffer) - 1,
                     0);

        if (n <= 0)
        {
            printf("Client%d disconnected.\n",
                   clientNumber);
            break;
        }

        buffer[n] = '\0';

        /*
         * Client sends exit
         */
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Client%d disconnected.\n",
                   clientNumber);

            break;
        }

        /*
         * Display client message
         */
        printf("Client%d: %s\n",
               clientNumber,
               buffer);

        /*
         * Server reply
         */
        printf("Server reply to Client%d: ",
               clientNumber);

        fgets(message,
              sizeof(message),
              stdin);

        message[strcspn(message, "\n")] = '\0';

        /*
         * Send server reply
         */
        send(clientfd,
             message,
             strlen(message),
             0);

        /*
         * Server ends chat
         */
        if (strcmp(message, "exit") == 0)
        {
            printf("Chat ended with Client%d.\n",
                   clientNumber);

            break;
        }
    }

    close(clientfd);

    exit(0);
}


int main()
{
    int sockfd;
    int clientfd;

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    socklen_t addrLen;

    /*
     * 1. Create TCP socket
     */
    sockfd = socket(AF_INET,
                    SOCK_STREAM,
                    0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    /*
     * Allow reuse of port
     */
    int opt = 1;

    setsockopt(sockfd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    memset(&serverAddr,
           0,
           sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;

    serverAddr.sin_port =
        htons(5000);

    serverAddr.sin_addr.s_addr =
        INADDR_ANY;

    /*
     * 2. Bind socket
     */
    if (bind(sockfd,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        printf("Bind failed\n");

        close(sockfd);

        return 1;
    }

    /*
     * 3. Listen for clients
     */
    if (listen(sockfd, 10) < 0)
    {
        printf("Listen failed\n");

        close(sockfd);

        return 1;
    }

    printf("Concurrent TCP Chat Server is running...\n");
    printf("Waiting for clients...\n");

    /*
     * 4. Accept clients continuously
     */
    while (1)
    {
        addrLen = sizeof(clientAddr);

        clientfd = accept(sockfd,
                          (struct sockaddr *)&clientAddr,
                          &addrLen);

        if (clientfd < 0)
        {
            printf("Accept failed\n");
            continue;
        }

        /*
         * Assign client number
         */
        clientCount++;

        /*
         * Create child process
         */
        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Fork failed\n");

            close(clientfd);

            continue;
        }

        /*
         * Child process
         */
        if (pid == 0)
        {
            close(sockfd);

            handle_client(clientfd,
                          clientCount);
        }

        /*
         * Parent process
         */
        else
        {
            close(clientfd);

            printf("New client accepted as Client%d\n",
                   clientCount);
        }
    }

    close(sockfd);

    return 0;
}
