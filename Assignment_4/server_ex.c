#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdint.h>

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int ttl = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int len, n;

        len = sizeof(cliaddr);
        n = recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);

        if (n <= 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        }

        ttl = buffer[sizeof(uint16_t) + sizeof(uint32_t)];

        if (ttl == 0) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            uint32_t recv_timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec;

            uint16_t seq_num = *((uint16_t *)buffer);
            uint32_t timestamp = *((uint32_t *)(buffer + sizeof(seq_num)));
            uint8_t ttl_val = *((uint8_t *)(buffer + sizeof(seq_num) + sizeof(timestamp)));

            double rtt = (double)(recv_timestamp - timestamp) / 1000.0;
            printf("Seq. number = %d, RTT = %.3lf ms\n", seq_num, rtt);
        } else {
            ttl--;
            buffer[sizeof(uint16_t) + sizeof(uint32_t)] = ttl;
            sendto(sockfd, (const char *)buffer, sizeof(buffer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
        }
    }

    return 0;
}
