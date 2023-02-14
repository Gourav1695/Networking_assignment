#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

struct client
{
    int sock;
    char name[BUFFER_SIZE];
};

struct client clients[MAX_CLIENTS];
pthread_t threads[MAX_CLIENTS];
int client_count = 0;

void *handle_client(void *arg)
{
    int i, j, k;
    struct client *c = (struct client *)arg;
    char buffer[BUFFER_SIZE];

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        read(c->sock, buffer, BUFFER_SIZE);

        if (strlen(buffer) == 0)
        {
            break;
        }

        for (i = 0; i < client_count; i++)
        {
            if (clients[i].sock != c->sock)
            {
                write(clients[i].sock, c->name, strlen(c->name));
                write(clients[i].sock, ": ", 2);
                write(clients[i].sock, buffer, strlen(buffer));
                write(clients[i].sock, "\n", 1);
            }
        }
    }

    close(c->sock);

    for (i = 0; i < client_count; i++)
    {
        if (clients[i].sock == c->sock)
        {
            for (j = i; j < client_count - 1; j++)
            {
                clients[j] = clients[j + 1];
            }
            break;
        }
    }

    client_count--;
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int sock, client_sock;
    struct sockaddr_in server, client;
    int c;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    listen(sock, MAX_CLIENTS);

    printf("Server started. Waiting for connections...\n");

    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        if (client_count >= MAX_CLIENTS)
        {
            write(client_sock, "Server is full. Please try again later.\n", 37);
            close(client_sock);
            continue;
        }

        clients[client_count].sock = client_sock;
        sprintf(clients[client_count].name, "%d", client_sock);
        pthread_create(&threads[client_count], NULL, handle_client, (void *)&clients[client_count]);
        client_count++;

        printf("Accepted connection from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    }

    if (client_sock < 0)
    {
        perror("Accept failed");
        return 1;
    }

    return 0;
}