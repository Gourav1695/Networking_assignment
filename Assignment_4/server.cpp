#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

const int MAX_PAYLOAD_SIZE = 1000;

int main(int argc, char * argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = atoi(argv[1]);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket.\n";
        return 1;
    }

    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "Error binding socket.\n";
        close(sockfd);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    socklen_t len;

    len = sizeof(cliaddr);

    while (true) {
        struct Packet {
            uint16_t sequence_number;
            uint32_t timestamp;
            uint8_t ttl;
            uint8_t payload[MAX_PAYLOAD_SIZE];
        } recv_packet;

        int n = recvfrom(sockfd, &recv_packet, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
        if (n < 0) {
            std::cerr << "Error receiving packet.\n";
            continue;
        }

        // Decrement TTL value by one
        recv_packet.ttl--;

        // Send the same packet back to the client
        if (sendto(sockfd, &recv_packet, sizeof(buffer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len) < 0) {
            perror("sendto");
            continue;
        }
    }

    close(sockfd);
    return 0;
}