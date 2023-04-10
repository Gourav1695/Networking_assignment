#include <iostream>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>

using namespace std;
using namespace chrono;

const int MAX_PAYLOAD_SIZE = 1400;

// Making the Packet structure
struct Packet
{
    int sequence_number;
    int timestamp;
    int ttl;
    int payload[MAX_PAYLOAD_SIZE];
};

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        cerr << "Usage: " << argv[0] << " <ServerIP> <ServerPort> <P> <TTL> <NumPackets>" << endl;
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    const int server_port = atoi(argv[2]);
    const int payload_size = atoi(argv[3]);
    const int ttl = atoi(argv[4]);
    const int num_packets = atoi(argv[5]);

    if (payload_size > MAX_PAYLOAD_SIZE)
    {
        cerr << "Error: Payload size too large" << endl;
        return EXIT_FAILURE;
    }
    if (ttl % 2 != 0)
    {
        cerr << "Error: TTL must be even" << endl;
        return EXIT_FAILURE;
    }

    // Create a datagram socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        cerr << "Error creating socket" << endl;
        return EXIT_FAILURE;
    }

    // Set server address and port
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_ip);
    servaddr.sin_port = htons(server_port);
    double total_rtt = 0;

    // Generate and send packets
    for (int i = 0; i < num_packets; i++)
    {
        // Generate packet
        Packet packet;
        packet.sequence_number = htons(i);
        packet.timestamp = htonl(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        packet.ttl = ttl;
        memset(packet.payload, 'A', payload_size);

        // Send packet to server
        if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            cerr << "Error sending packet" << endl;
            return EXIT_FAILURE;
        }

        // Receive packet from server
        Packet recv_packet;
        struct sockaddr_in recvaddr;
        socklen_t len = sizeof(recvaddr);
        if (recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&recvaddr, &len) < 0)
        {
            cerr << "Error receiving packet" << endl;
            return EXIT_FAILURE;
        }

        // Calculate RTT delay
        int seq_num = ntohs(recv_packet.sequence_number);
        int sent_time = ntohl(recv_packet.timestamp);
        int recv_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        double rtt_delay = static_cast<double>(recv_time - sent_time) / 1000.0;
        int recv_ttl = recv_packet.ttl;
        cout << "Packet No. " << seq_num << ", RTT delay: " << rtt_delay << " ms"
             << ", Recieved TTL: " << static_cast<int>(recv_ttl) << endl;
        total_rtt = total_rtt + rtt_delay;
    }
    // Print summary statistics
    double average_rtt = total_rtt / (double)num_packets;
    cout << "Summary:" << endl;
    cout << "  Number of Packets: " << num_packets << endl;
    cout << "  Payload Size: " << payload_size << endl;
    cout << "  TTL: " << ttl << endl;
    cout << "  Average RTT: " << average_rtt << " ms" << endl;

    close(sockfd);
    return EXIT_SUCCESS;
}