#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sockfd, n;
    struct sockaddr_in server_addr;
    char filename[100];
    char buffer[BUFFER_SIZE];
    FILE *fp;

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        close(sockfd);
        exit(1);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sockfd);
        exit(1);
    }

    printf("Connected to server.\n");
    printf("Enter filename to request: ");
    scanf("%s", filename);

    // Send filename to server
    send(sockfd, filename, strlen(filename), 0);

    // Receive server response ("OK" or "NO")
    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0)
    {
        printf("No response from server.\n");
        close(sockfd);
        return 0;
    }

    buffer[n] = '\0';
    if (strcmp(buffer, "NO") == 0)
    {
        printf("File not found on server.\n");
        close(sockfd);
        return 0;
    }

    // Open local file to save incoming data
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        printf("Cannot create local file.\n");
        close(sockfd);
        return 0;
    }

    // Receive file chunks and write to disk
    while ((n = recv(sockfd, buffer, BUFFER_SIZE, 0)) > 0)
    {
        fwrite(buffer, 1, n, fp);
    }

    printf("File received successfully.\n");
    fclose(fp);
    close(sockfd);

    return 0;
}
