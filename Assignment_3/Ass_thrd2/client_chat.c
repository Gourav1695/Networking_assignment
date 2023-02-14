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
    int sockfd = *(int *)arg; 
    char buff[MAX]; 
    int n; 
    while (1) { 
        bzero(buff, MAX); 
        printf("Enter the string : "); 
        n = 0; 
        while ((buff[n++] = getchar()) != '\n') 
            ; 
        write(sockfd, buff, sizeof(buff)); 
        bzero(buff, MAX); 
        read(sockfd, buff, sizeof(buff)); 
        printf("From Server : %s", buff); 
        if ((strncmp(buff, "exit", 4)) == 0) { 
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
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT); 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); //  program has been executed without any error or interrupt
    } 
    else
        printf("connected to the server..\n"); 
    pthread_t tid;
    int *p = malloc(sizeof(int));
    *p = sockfd;
    pthread_create(&tid, NULL, thread_function, p);
    pthread_join(tid, NULL);
    close(sockfd); 
}
