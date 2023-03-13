// 2020CSB010 GOURAV KUMAR SHAW

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

void *receive_thread_function(void *arg)
{
    int sockfd = *(int *)arg;

    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, MAX);

        read(sockfd, buff, sizeof(buff));
        printf("From server: %s\n", buff);
        if (strncmp("exit", buff, 4) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
}

void *send_thread_function(void *arg)
{
    int sockfd = *(int *)arg;

    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));
        if (strncmp("exit", buff, 4) == 0)
        {
            printf("Client Exit...\n");
            break;
        }
    }
}

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    pthread_t receive_tid, send_tid;
    int *p = malloc(sizeof(int));
    *p = sockfd;
    pthread_create(&receive_tid, NULL, receive_thread_function, p);
    pthread_create(&send_tid, NULL, send_thread_function, p);
    pthread_join(receive_tid, NULL);
    pthread_join(send_tid, NULL);
    close(sockfd);
}

