// 2020CSB010 GOURAV KUMAR SHAW

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);// sock_stream means TCP , 0 is IP (int sockfd = socket(domain, type, protocol))
    if (sock == -1) {
        perror("Could not create socket");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");// local address
    server.sin_family = AF_INET;// AF_INET FOR IPv4
    server.sin_port = htons(PORT);// unsigned short int htons(unsigned short int hostshort);//host byte order to network byte order

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Enter message: ");
    scanf("%s", message);

    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        return 1;
    }



    if (recv(sock, message, BUFFER_SIZE, 0) < 0) {
        perror("Recv failed");
        return 1;
    }

    printf("Client received message: %s\n", message);

    close(sock);

    return 0;
}
