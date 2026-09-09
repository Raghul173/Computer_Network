#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    // Listen for connections
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Concurrent FTP Server is listening on port %d...\n", PORT);

    while (1)
    {
        addr_len = sizeof(client_addr);
        new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_sock < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected.\n");

        // Create a child process to handle the client concurrently
        if (fork() == 0)
        {
            // --- Child Process ---
            close(server_fd); // Child doesn't need the listening socket[cite: 3]

            char filename[100];
            char buffer[BUFFER_SIZE];
            FILE *fp;
            int n;

            // Receive requested filename from client
            n = recv(new_sock, filename, sizeof(filename) - 1, 0);
            if (n > 0)
            {
                filename[n] = '\0';
                printf("Requested file: %s\n", filename);

                fp = fopen(filename, "rb");
                if (fp == NULL)
                {
                    printf("File not found.\n");
                    send(new_sock, "NO", 2, 0);
                }
                else
                {
                    send(new_sock, "OK", 2, 0);
                    // Read file and send chunks to the client
                    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0)
                    {
                        send(new_sock, buffer, n, 0);
                    }
                    printf("File sent successfully.\n");
                    fclose(fp);
                }
            }

            // Close client connection for this child and exit[cite: 3]
            close(new_sock);
            printf("Client disconnected.\n");
            exit(0);
        }

        // --- Parent Process ---
        // Parent closes its reference to the connected socket and continues looping[cite: 3]
        close(new_sock);
    }

    close(server_fd);
    return 0;
}
