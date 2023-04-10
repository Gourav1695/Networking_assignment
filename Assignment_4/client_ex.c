#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_PAYLOAD_SIZE 1000
#define MAX_TTL 20
#define MIN_TTL 2
#define TTL_STEP 2
#define NUM_DATAGRAMS 50

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("Usage: %s <server-ip> <payload-size> <output-file>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int payload_size = atoi(argv[2]);
    char *output_file = argv[3];

    if (payload_size < 100 || payload_size > 1000)
    {
        printf("Payload size should be between 100 and 1000 bytes\n");
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    if (inet_aton(server_ip, &server_addr.sin_addr) == 0)
    {
        printf("Invalid IP address\n");
        exit(1);
    }

    char send_buf[sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t) + MAX_PAYLOAD_SIZE];
    struct timeval start_time;
    double total_rtt = 0;

    FILE *fp = fopen(output_file, "w");
    if (fp == NULL)
    {
        perror("Error opening output file");
        exit(1);
    }

    for (int ttl = MIN_TTL; ttl <= MAX_TTL; ttl += TTL_STEP)
    {
        memset(&send_buf, 0, sizeof(send_buf));
        uint16_t seq_num;

        for (int i = 0; i < NUM_DATAGRAMS; i++)
        {
            seq_num = i;
            uint32_t timestamp;
            struct timeval current_time;

            gettimeofday(&start_time, NULL);

            gettimeofday(&current_time, NULL);
            timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec;
            uint8_t ttl_val = ttl;
            char payload[payload_size];
            memset(payload, 'a', sizeof(payload));

            memcpy(send_buf, &seq_num, sizeof(seq_num));
            memcpy(send_buf + sizeof(seq_num), &timestamp, sizeof(timestamp));
            memcpy(send_buf + sizeof(seq_num) + sizeof(timestamp), &ttl_val, sizeof(ttl_val));
            memcpy(send_buf + sizeof(seq_num) + sizeof(timestamp) + sizeof(ttl_val), payload, payload_size);

            if (sendto(sockfd, send_buf, sizeof(send_buf), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                perror("Error sending datagram");
                exit(1);
            }

            char recv_buf[1024];
            struct sockaddr_in recv_addr;
            socklen_t addr_len = sizeof(recv_addr);
            memset(recv_buf, 0, sizeof(recv_buf));
            if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len) < 0)
            {
                perror("Error receiving datagram");
                exit(1);
            }
            uint16_t recv_seq_num;
            uint32_t recv_timestamp;
            uint8_t recv_ttl;

            memcpy(&recv_seq_num, recv_buf, sizeof(recv_seq_num));
            memcpy(&recv_timestamp, recv_buf + sizeof(recv_seq_num), sizeof(recv_timestamp));
            memcpy(&recv_ttl, recv_buf + sizeof(recv_seq_num) + sizeof(recv_timestamp), sizeof(recv_ttl));

            if (recv_seq_num != seq_num)
            {
                printf("Received out-of-order packet. Expected seq_num=%d, but received seq_num=%d\n", seq_num, recv_seq_num);
                continue;
            }

            struct timeval end_time;
            gettimeofday(&end_time, NULL);

            double rtt = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
            total_rtt += rtt;

            fprintf(fp, "%d %lf\n", ttl, rtt);

            if (recv_ttl == 0)
            {
                printf("Received response from server. Exiting...\n");
                fclose(fp);
                close(sockfd);
                exit(0);
            }
        }
    }

    double avg_rtt = total_rtt / (NUM_DATAGRAMS * (MAX_TTL - MIN_TTL + TTL_STEP) / TTL_STEP);
    printf("Average RTT: %lf ms\n", avg_rtt);

    fclose(fp);
    close(sockfd);
    return 0;
}