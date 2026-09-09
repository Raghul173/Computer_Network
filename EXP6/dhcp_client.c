#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 5000
#define SERVER_IP "127.0.0.1"


/* -------------------------------------------------
   Integer Input Function
   ------------------------------------------------- */

int get_int_input(const char *prompt)
{
    int value;
    char line[100];

    while (1)
    {
        printf("%s", prompt);

        if (fgets(line,
                  sizeof(line),
                  stdin) == NULL)
        {
            printf("\nExiting due to input stream closure.\n");
            exit(0);
        }

        if (sscanf(line, "%d", &value) == 1)
        {
            return value;
        }

        printf("Invalid input. Please enter a valid integer.\n");
    }
}


/* -------------------------------------------------
   MAIN
   ------------------------------------------------- */

int main()
{
    int sockfd;

    char message[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serverAddr;

    socklen_t addrLen =
        sizeof(serverAddr);

    /* Create UDP socket */
    sockfd = socket(AF_INET,
                    SOCK_DGRAM,
                    0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr,
           0,
           sizeof(serverAddr));

    serverAddr.sin_family =
        AF_INET;

    serverAddr.sin_port =
        htons(SERVER_PORT);

    serverAddr.sin_addr.s_addr =
        inet_addr(SERVER_IP);


    printf("--- DHCP Client (Iterative) ---\n");

    printf("Connecting to DHCP Server at %s:%d\n\n",
           SERVER_IP,
           SERVER_PORT);


    /* -------------------------------------------------
       PHASE 1 : SETUP
       ------------------------------------------------- */

    int total =
        get_int_input(
            "Enter total number of IP addresses: ");

    int subnets =
        get_int_input(
            "Enter number of blocks: ");


    /*
     * Send SETUP request
     */
    sprintf(message,
            "SETUP %d %d",
            total,
            subnets);


    if (sendto(sockfd,
               message,
               strlen(message),
               0,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr)) < 0)
    {
        perror("Sendto failed (SETUP)");

        close(sockfd);

        return 1;
    }


    /*
     * Receive SETUP response
     */
    int n =
        recvfrom(sockfd,
                 buffer,
                 BUFFER_SIZE - 1,
                 0,
                 (struct sockaddr *)&serverAddr,
                 &addrLen);


    if (n < 0)
    {
        perror("Receive failed (SETUP)");

        close(sockfd);

        return 1;
    }


    buffer[n] = '\0';


    printf("\n--- Server Response (SETUP) ---\n");

    printf("%s\n",
           buffer);


    /* -------------------------------------------------
       PHASE 2 : ALLOCATION
       ------------------------------------------------- */

    while (1)
    {
        printf("----------------------------\n");

        int block =
            get_int_input(
                "Enter block number (0 to exit): ");


        /*
         * EXIT
         */
        if (block == 0)
        {
            strcpy(message,
                   "EXIT");

            printf("Sending EXIT message...\n");


            sendto(sockfd,
                   message,
                   strlen(message),
                   0,
                   (struct sockaddr *)&serverAddr,
                   sizeof(serverAddr));


            /*
             * Receive server confirmation
             */
            n =
                recvfrom(sockfd,
                         buffer,
                         BUFFER_SIZE - 1,
                         0,
                         (struct sockaddr *)&serverAddr,
                         &addrLen);


            if (n >= 0)
            {
                buffer[n] = '\0';

                printf("Server Disconnect Msg: %s\n",
                       buffer);
            }

            break;
        }


        /*
         * Number of IPs required
         */
        int required =
            get_int_input(
                "Enter number of IP addresses to allot: ");


        /*
         * Send ALLOCATE request
         */
        sprintf(message,
                "ALLOCATE %d %d",
                block,
                required);


        if (sendto(sockfd,
                   message,
                   strlen(message),
                   0,
                   (struct sockaddr *)&serverAddr,
                   sizeof(serverAddr)) < 0)
        {
            perror("Sendto failed (ALLOCATE)");

            continue;
        }


        /*
         * Receive allocation response
         */
        n =
            recvfrom(sockfd,
                     buffer,
                     BUFFER_SIZE - 1,
                     0,
                     (struct sockaddr *)&serverAddr,
                     &addrLen);


        if (n < 0)
        {
            perror("Receive failed (ALLOCATE)");

            continue;
        }


        buffer[n] = '\0';


        printf("\n--- Server Response (ALLOCATE) ---\n");

        printf("%s\n",
               buffer);
    }


    /* Close socket */
    close(sockfd);

    printf("Client socket closed. Goodbye.\n");

    return 0;
}
