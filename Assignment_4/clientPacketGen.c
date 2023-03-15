#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 1000

void error_handling(char *message);

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("Usage: %s <IP> <port> <P> <TTL> <NumPackets>\n", argv[0]);
        exit(1);
    }

    srand(time(NULL));

    int sock;
    char message[BUF_SIZE];
    struct sockaddr_in serv_addr;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock == -1)
        error_handling("UDP socket creation error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    int p = atoi(argv[3]);
    if (p < 100 || p > 1000)
    {
        printf("P value should be within 100 to 1000 bytes\n");
        exit(1);
    }

    int ttl = atoi(argv[4]);
    if (ttl < 2 || ttl > 20 || ttl % 2 != 0)
    {
        printf("TTL value should be within 2 to 20 (and must be even)\n");
        exit(1);
    }

    int num_packets = atoi(argv[5]);
    if (num_packets < 1 || num_packets > 50)
    {
        printf("NumPackets value should be within 1 to 50\n");
        exit(1);
    }

    for (int i = 0; i < num_packets; i++)
    {
        memset(message, 0, BUF_SIZE);
        sprintf(message, "%d", i);
        for (int j = 0; j < p - strlen(message) - 1; j++)
        {
            message[strlen(message)] = rand() % 26 + 'a';
        }
        message[p - 1] = '\0';
        int ttl_option = ttl;
        setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl_option, sizeof(ttl_option));
        sendto(sock, message, strlen(message) + 1, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        recvfrom(serv_sock, message, BUF_SIZE, 0, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        sleep(1);
    }

    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}