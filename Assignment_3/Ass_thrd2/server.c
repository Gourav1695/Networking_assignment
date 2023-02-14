// 2020CSB010 GOURAV KUMAR SHAW

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

void *connection_handler(void *);

int main(int argc, char *argv[])
{
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);// The first argument, AF_INET, specifies that the socket should use the IPv4 protocol. The second argument, SOCK_STREAM, specifies that the socket should use the TCP protocol, which provides a reliable, stream-oriented connection. The third argument, 0, specifies a default protocol to be used
    if (socket_desc == -1)
    {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT); // unsigned short int htons(unsigned short inthostshort);

    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Bind failed");
        return 1;
    }

    listen(socket_desc, 3); // int listen(int sockfd, int backlog); here 3 is backlog means The backlog, defines the maximum length to which the queue of pending connections for sockfd may grow.

    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);

    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        printf("Connection accepted\n");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        // 1.When a client connects, the accept function returns a new socket descriptor for communication with the client. The client structure is filled with the client's address information.
        // 2.For each new connection, a new thread is created using pthread_create. The thread function connection_handler is passed the new_sock parameter, which is a pointer to the newly created socket descriptor for communication with the client.
        // 3.The malloc function is used to allocate memory for the new_sock variable. This memory is then passed to the thread function as a parameter.
        // 4.The main thread continues to wait for new connections and repeat the process of creating a new thread for each connection.
        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
        {
            perror("Could not create thread");
            return 1;
        }

        printf("Handler assigned\n");
    }

    if (client_sock < 0)
    {
        perror("Accept failed");
        return 1;
    }

    return 0;
}

void *connection_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    int read_size;
    char message[BUFFER_SIZE];

    while ((read_size = recv(sock, message, BUFFER_SIZE, 0)) > 0)
    {
        printf("Server received message: %s\n", message);

        if (send(sock, message, strlen(message), 0) < 0)
        {
            perror("Send failed");
            return 0;
        }
    }

    if (read_size == 0)
    {
        printf("Client disconnected\n");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("Recv failed");
    }

    free(socket_desc);

    return 0;
}
