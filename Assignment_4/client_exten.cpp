#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

int main(int argc, char *argv[])
{

    if (argc != 6)
    {
        cerr << "Usage: " << argv[0] << " <ServerIP> <ServerPort> <P> <TTL> <NumPackets>" << endl;
        exit(EXIT_FAILURE);
    }

    // Extract command-line arguments
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int payload_size = atoi(argv[3]);
    int ttl = atoi(argv[4]);
    int num_packets = atoi(argv[5]);

    if (payload_size < 100 || payload_size > 1000)
    {
        cerr << "Payload size must be between 100 and 1000 bytes." << endl;
        exit(EXIT_FAILURE);
    }

    if (ttl < 2 || ttl > 20 || ttl % 2 != 0)
    {
        cerr << "TTL must be between 2 and 20 and must be even." << endl;
        exit(EXIT_FAILURE);
    }

    if (num_packets < 1 || num_packets > 50)
    {
        cerr << "Number of packets must be between 1 and 50." << endl;
        exit(EXIT_FAILURE);
    }

    // Create a UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   
    if (sockfd < 0)
    {
        cerr << "Failed to create socket." << endl;
        exit(EXIT_FAILURE);
    }

    // Set server address
    struct sockaddr_in server_addr;
   
    memset(&server_addr, 0, sizeof(server_addr)); 

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);
   
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    /*The first line creates a struct timeval object called start_time and initializes it to the current time using the gettimeofday() function.*/
    double total_rtt = 0;

    // Send packets and receive response
    for (int i = 0; i < num_packets; i++)
    {
        // Generate packet
        uint16_t seq_num = i;
        uint32_t timestamp;
        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec; 
        uint8_t ttl_val = ttl;
        char payload[payload_size];
        memset(payload, 'a', sizeof(payload)); /*Overall, this code snippet prepares the necessary data required to send a UDP packet, including a sequence number, timestamp, TTL value, and payload data.*/

        // Send packet to server
        sendto(sockfd, &seq_num, sizeof(seq_num), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, &timestamp, sizeof(timestamp), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, &ttl_val, sizeof(ttl_val), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, payload, sizeof(payload), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Receive response from server
        char recv_buf[1024];
        struct sockaddr_in recv_addr;
        socklen_t addr_len = sizeof(recv_addr);
        recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len);

        while(ttl_val>0){
        if (ttl_val > 0) {
        ttl_val--;
        *((uint8_t*)(recv_buf + sizeof(uint16_t) + sizeof(uint32_t))) = ttl_val;
        sendto(sockfd, &seq_num, sizeof(seq_num), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, &timestamp, sizeof(timestamp), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, &ttl_val, sizeof(ttl_val), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(sockfd, payload, sizeof(payload), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        }
        
    }
        // Calculate RTT
        uint32_t recv_timestamp;
        gettimeofday(&current_time, NULL);

        recv_timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec;
        uint16_t recv_seq_num = *((uint16_t *)recv_buf);
        memcpy(&recv_timestamp, recv_buf + sizeof(recv_seq_num), sizeof(recv_timestamp)); 
        //void* memset( void* str, int ch, size_t n);
       
        uint8_t recv_ttl = *((uint8_t *)(recv_buf + sizeof(recv_seq_num) + sizeof(recv_timestamp)));
        double rtt = (double)(recv_timestamp - timestamp) / 1000.0;
        total_rtt += rtt;
        

        // Print packet information
        cout << "Packet " << i << ":" << endl;
        cout << "  Sequence Number: " << seq_num << endl;
        cout << "  Time Sent: " << timestamp << endl;
        cout << "  TTL: " << (int)ttl_val << endl;
        cout << "  Payload Size: " << payload_size << endl;
        cout << "  RTT: " << rtt << " ms" << endl;
        cout << endl;

        usleep(50000); // Sleep for 50ms between packets
    }

    // Print summary statistics
    double average_rtt = total_rtt / (double)num_packets;
    cout << "Summary:" << endl;
    cout << "  Num Packets: " << num_packets << endl;
    cout << "  Payload Size: " << payload_size << endl;
    cout << "  TTL: " << ttl << endl;
    cout << "  Average RTT: " << average_rtt << " ms" << endl;

    // Close the socket
    close(sockfd);

    return 0;
}