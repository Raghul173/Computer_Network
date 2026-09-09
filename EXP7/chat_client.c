#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sockfd;

    char message[1024];
    char buffer[1024];
    char clientName[50];

    struct sockaddr_in serverAddr;

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

    memset(&serverAddr,
           0,
           sizeof(serverAddr));

    serverAddr.sin_family =
        AF_INET;

    serverAddr.sin_port =
        htons(5000);

    serverAddr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    /*
     * 2. Connect to server
     */
    if (connect(sockfd,
                (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) < 0)
    {
        printf("Connection failed\n");

        close(sockfd);

        return 1;
    }

    /*
     * Receive client number
     */
    int n = recv(sockfd,
                 buffer,
                 sizeof(buffer) - 1,
                 0);

    if (n <= 0)
    {
        printf("Server disconnected\n");

        close(sockfd);

        return 1;
    }

    buffer[n] = '\0';

    strcpy(clientName,
           buffer);

    printf("Connected to Chat Server as %s.\n",
           clientName);

    printf("Type 'exit' to quit.\n");

    /*
     * 3. Chat loop
     */
    while (1)
    {
        /*
         * Enter message
         */
        printf("%s: ",
               clientName);

        fgets(message,
              sizeof(message),
              stdin);

        message[strcspn(message, "\n")] = '\0';

        /*
         * Send message
         */
        send(sockfd,
             message,
             strlen(message),
             0);

        /*
         * Client exits
         */
        if (strcmp(message, "exit") == 0)
        {
            break;
        }

        /*
         * Receive server reply
         */
        n = recv(sockfd,
                 buffer,
                 sizeof(buffer) - 1,
                 0);

        if (n <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[n] = '\0';

        /*
         * Server exits
         */
        if (strcmp(buffer, "exit") == 0)
        {
            printf("Server has ended the chat.\n");
            break;
        }

        printf("Server: %s\n",
               buffer);
    }

    /*
     * Close socket
     */
    close(sockfd);

    printf("Client socket closed.\n");

    return 0;
}
