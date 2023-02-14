#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SERV_TCP_PORT 5035
int main(int argc, char **argv)
{
    int sockfd, newsockfd, clength;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[4096];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_TCP_PORT);
    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(sockfd, 5);
    clength = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clength);
    read(newsockfd, buffer, 4096);
    while (buffer != "quit")
    {
        printf("\nClient message: %s", buffer);
        printf("\nType your message : ");
        fgets(buffer, 4096, stdin);
        write(newsockfd, buffer, 4096);
        printf("\n");
        read(newsockfd, buffer, 4096);
    }
    close(sockfd);
    return 0;
}
