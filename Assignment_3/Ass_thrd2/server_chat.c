#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void *thread_function(void *arg)
{
    int sockfd = *(int *)arg; // The argument passed to the function is expected to be a pointer to an integer, which is then dereferenced to obtain the actual socket file descriptor value.

    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, MAX); // in this case "MAX". This function can be used to clear the contents of a buffer before using it, ensuring that there are no leftover data from previous operations.

        read(sockfd, buff, sizeof(buff));
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));
        if (strncmp("exit", buff, 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
    }
}

int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // The first argument, AF_INET, specifies that the socket should use the IPv4 protocol. The second argument, SOCK_STREAM, specifies that the socket should use the TCP protocol, which provides a reliable, stream-oriented connection. The third argument, 0, specifies a default protocol to be used
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // The function htonl is the host-to-network long function, which converts a 32-bit integer from host byte order to network byte order. By setting servaddr.sin_addr.s_addr to htonl(INADDR_ANY), the socket is being told to listen on all available network interfaces and to use any IP address associated with them.
    servaddr.sin_port = htons(PORT);              // The function htons is the host-to-network short function, which converts the port number from host byte order to network byte order, ensuring that the value is stored in the correct byte order for network communication.
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully bound..\n");
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");
    pthread_t tid;
    int *p = malloc(sizeof(int));
    *p = connfd;
    pthread_create(&tid, NULL, thread_function, p);
    pthread_join(tid, NULL);
    close(sockfd);
}