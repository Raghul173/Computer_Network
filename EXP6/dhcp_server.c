#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MAX_SUBNETS 10
#define MAX_IPS 256

int subnet_count;
int ip1, ip2, ip3, ip4;

int total_ips[MAX_SUBNETS];
int used[MAX_SUBNETS][MAX_IPS];

void process_setup(int sockfd, struct sockaddr_in clientAddr,
                   socklen_t addrLen, char *message);

void process_allocate(int sockfd, struct sockaddr_in clientAddr,
                      socklen_t addrLen, char *message);

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen;

    srand(time(NULL));

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    /* Bind socket */
    if (bind(sockfd,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("UDP Iterative DHCP Server is running...\n");

    while (1)
    {
        int n;

        addrLen = sizeof(clientAddr);

        /* Receive message */
        n = recvfrom(sockfd,
                     buffer,
                     sizeof(buffer) - 1,
                     0,
                     (struct sockaddr *)&clientAddr,
                     &addrLen);

        if (n < 0)
        {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';

        printf("\nClient: %s\n",
               inet_ntoa(clientAddr.sin_addr));

        /* SETUP request */
        if (strncmp(buffer, "SETUP", 5) == 0)
        {
            process_setup(sockfd,
                          clientAddr,
                          addrLen,
                          buffer);
        }

        /* ALLOCATE request */
        else if (strncmp(buffer, "ALLOCATE", 8) == 0)
        {
            process_allocate(sockfd,
                             clientAddr,
                             addrLen,
                             buffer);
        }

        /* EXIT request */
        else if (strcmp(buffer, "EXIT") == 0)
        {
            char response[] =
                "DHCP client disconnected.";

            sendto(sockfd,
                   response,
                   strlen(response),
                   0,
                   (struct sockaddr *)&clientAddr,
                   addrLen);

            printf("Client disconnected\n");
        }

        else
        {
            char response[] =
                "Invalid request.";

            sendto(sockfd,
                   response,
                   strlen(response),
                   0,
                   (struct sockaddr *)&clientAddr,
                   addrLen);
        }
    }

    close(sockfd);

    return 0;
}


/* -------------------------------------------------
   SETUP FUNCTION
   ------------------------------------------------- */

void process_setup(int sockfd,
                   struct sockaddr_in clientAddr,
                   socklen_t addrLen,
                   char *message)
{
    char response[BUFFER_SIZE];

    int total;
    int required_subnets;

    int i, j;
    int power = 0;
    int power_value = 1;

    int base;
    int remainder;

    /* Read input */
    if (sscanf(message,
               "SETUP %d %d",
               &total,
               &required_subnets) != 2)
    {
        strcpy(response,
               "Invalid SETUP format.");
    }

    else if (required_subnets < 1 ||
             required_subnets > MAX_SUBNETS)
    {
        sprintf(response,
                "Invalid number of blocks (1-%d).",
                MAX_SUBNETS);
    }

    else if (total <= 0)
    {
        strcpy(response,
               "Invalid total number of IP addresses.");
    }

    else
    {
        /*
         * Find the next power of 2.
         *
         * Example:
         * 10  -> 16  -> 2^4
         * 50  -> 64  -> 2^6
         * 100 -> 128 -> 2^7
         */

        while (power_value < total)
        {
            power_value = power_value * 2;
            power++;
        }

        /*
         * Maximum supported IP addresses
         */
        if (power_value > MAX_IPS)
        {
            strcpy(response,
                   "Number of IP addresses exceeds maximum 256.");
        }

        else
        {
            subnet_count = required_subnets;

            /*
             * Generate random network block
             */
            ip1 = 192;
            ip2 = 168;
            ip3 = 1 + rand() % 254;
            ip4 = 0;

            /*
             * Divide power-of-2 IP block
             * among the requested blocks.
             */
            base = power_value / subnet_count;
            remainder = power_value % subnet_count;

            sprintf(response,
                    "DHCP Server Setup\n\n"
                    "Requested IP Addresses: %d\n"
                    "Using nearest power of 2: 2^%d = %d\n"
                    "Random IP Block: %d.%d.%d.%d\n"
                    "Total IP Addresses Used: %d\n"
                    "Number of Blocks: %d\n\n",
                    total,
                    power,
                    power_value,
                    ip1,
                    ip2,
                    ip3,
                    ip4,
                    power_value,
                    subnet_count);

            /*
             * Start address of first block
             */
            int current_ip_start = 1;

            for (i = 0; i < subnet_count; i++)
            {
                /*
                 * Divide addresses among blocks.
                 */
                total_ips[i] = base;

                if (i < remainder)
                {
                    total_ips[i]++;
                }

                /*
                 * Calculate end address
                 */
                int end_addr =
                    current_ip_start +
                    total_ips[i] - 1;

                /*
                 * Reset used array
                 */
                for (j = 0; j < MAX_IPS; j++)
                {
                    used[i][j] = 0;
                }

                /*
                 * Display block information
                 */
                char temp[250];

                sprintf(temp,
                        "Block %d:\n"
                        "Start Address: %d.%d.%d.%d\n"
                        "End Address: %d.%d.%d.%d\n"
                        "Available IP Addresses: %d\n\n",

                        i + 1,

                        ip1,
                        ip2,
                        ip3,
                        current_ip_start,

                        ip1,
                        ip2,
                        ip3,
                        end_addr,

                        total_ips[i]);

                strcat(response, temp);

                /*
                 * Move to next block
                 */
                current_ip_start += total_ips[i];
            }
        }
    }

    printf("%s", response);

    sendto(sockfd,
           response,
           strlen(response),
           0,
           (struct sockaddr *)&clientAddr,
           addrLen);
}


/* -------------------------------------------------
   ALLOCATE FUNCTION
   ------------------------------------------------- */

void process_allocate(int sockfd,
                      struct sockaddr_in clientAddr,
                      socklen_t addrLen,
                      char *message)
{
    char response[BUFFER_SIZE];

    int subnet;
    int required;

    int i;
    int count = 0;

    if (sscanf(message,
               "ALLOCATE %d %d",
               &subnet,
               &required) != 2)
    {
        strcpy(response,
               "Invalid ALLOCATE format.");
    }

    else if (subnet < 1 ||
             subnet > subnet_count)
    {
        sprintf(response,
                "Invalid block number (1-%d).",
                subnet_count);
    }

    else if (required <= 0)
    {
        strcpy(response,
               "Invalid number of IP addresses requested.");
    }

    else
    {
        int subnet_idx = subnet - 1;

        if (required > total_ips[subnet_idx])
        {
            sprintf(response,
                    "Not enough IP addresses available "
                    "in this block. Only %d left.",
                    total_ips[subnet_idx]);
        }

        else
        {
            strcpy(response,
                   "IP addresses allotted successfully:\n\n");

            /*
             * Calculate starting IP of selected block
             */
            int start_ip = 1;

            int k;

            for (k = 0; k < subnet_idx; k++)
            {
                start_ip += total_ips[k];
            }

            /*
             * Allocate IP addresses
             */
            for (i = 0;
                 i < MAX_IPS && count < required;
                 i++)
            {
                if (used[subnet_idx][i] == 0)
                {
                    char temp[50];

                    used[subnet_idx][i] = 1;

                    sprintf(temp,
                            "%d.%d.%d.%d\n",
                            ip1,
                            ip2,
                            ip3,
                            start_ip + i);

                    strcat(response, temp);

                    count++;
                }
            }

            /*
             * Update remaining IP addresses
             */
            total_ips[subnet_idx] -= count;
        }
    }

    printf("%s\n", response);

    sendto(sockfd,
           response,
           strlen(response),
           0,
           (struct sockaddr *)&clientAddr,
           addrLen);
}
