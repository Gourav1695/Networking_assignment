#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUF_SIZE 1024

struct client_info {
    int sock;
    char name[BUF_SIZE];
};

struct client_info clients[MAX_CLIENTS];
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    struct client_info *client = (struct client_info *)arg;
    char buf[BUF_SIZE];

    while (1) {
        int received = recv(client->sock, buf, BUF_SIZE, 0);
        if (received <= 0) {
            break;
        }

        buf[received] = '\0';

        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock != client->sock) {
                if (send(clients[i].sock, buf, strlen(buf), 0) < 0) {
                    perror("send");
                }
            }
        }
        pthread_mutex_unlock(&client_mutex);
    }

    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == client->sock) {
            clients[i].sock = -1;
            strcpy(clients[i].name, "");
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    close(client->sock);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        return 1;
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    int port = atoi(argv[1]);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    // if (listen(server_listen(server_sock, 5)));

    memset(clients, -1, sizeof(clients));

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("accept");
            return 1;
        }

        pthread_mutex_lock(&client_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock == -1) {
                clients[i].sock = client_sock;
                sprintf(clients[i].name, "%d", client_sock);
                break;
            }
        }
        pthread_mutex_unlock(&client_mutex);

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, &clients[client_sock]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    close(server_sock);

    return 0;
}

