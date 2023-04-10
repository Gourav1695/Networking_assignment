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
    /*
    The first argument AF_INET specifies that the socket will be used for IP version 4 communication. The second argument SOCK_DGRAM specifies that this is a datagram socket, which means it will use the User Datagram Protocol (UDP) instead of the Transmission Control Protocol (TCP). The third argument 0 indicates that the default protocol for the selected domain and socket type will be used.

    If the socket creation is successful, the function returns a non-negative integer file descriptor that can be used to refer to the socket in other function calls. If there is an error, the function returns -1.

    */
    if (sockfd < 0)
    {
        cerr << "Failed to create socket." << endl;
        exit(EXIT_FAILURE);
    }

    // Set server address
    struct sockaddr_in server_addr;
    /*
    struct sockaddr_in is a structure defined in the netinet/in.h header file in C/C++ that represents an internet endpoint address, which includes the IP address and port number. It is used for IP communication with IPv4.
    */
    memset(&server_addr, 0, sizeof(server_addr)); // memset() is a C library function that sets a block of memory to a particular value. In this case, it's being used to set all bytes in the server_addr struct to 0.

    // The server_addr struct is used to hold information about the server that the client will be communicating with over the network. It's defined as a sockaddr_in structure, which is a structure used by the networking functions in C to store IP address and port information.

    // By setting all bytes in server_addr to 0 using memset(), the function is effectively initializing all the fields in the struct to their default values. This is a common practice to ensure that the struct is properly initialized before it's used, as some fields might not be set by the program and may contain arbitrary values from memory.

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(server_port);
    /*
    In this code, server_addr is a variable of type struct sockaddr_in that represents the server address the client will connect to. The sin_family member is set to AF_INET to indicate that the address is an IPv4 address. The sin_addr.s_addr member is set to the IP address of the server in network byte order using the inet_addr() function. The sin_port member is set to the port number of the server in network byte order using the htons() function.
    */
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
        timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec; /*The code current_time.tv_sec returns the number of seconds since the Epoch (January 1st, 1970, 00:00:00 UTC) until the current time, while current_time.tv_usec returns the number of microseconds (1/1,000,000 of a second) elapsed since the last second.

        So to get a timestamp in microseconds, we multiply the number of seconds by 1,000,000 and add the number of microseconds. This is what the line timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec; does.

        The resulting timestamp represents the number of microseconds that have elapsed since the Epoch until the current time.
 */
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
        /*reads incoming data from the socket identified by the file descriptor sockfd.

        The received data is stored in the character array recv_buf, which has a size of 1024 bytes.

        The details of the sender of the received data are stored in the recv_addr variable of the sockaddr_in type, which is a structure that contains information about an Internet address.

        The addr_len variable is used to store the size of the address structure pointed to by recv_addr.

        The recvfrom() function is used to read data from a socket. Its arguments are:

        sockfd: The file descriptor of the socket to read from
        recv_buf: A pointer to the buffer where the received data will be stored
        sizeof(recv_buf): The maximum size of the buffer in bytes
        0: Flags to modify the behavior of the function (in this case, no flags are set)
        (struct sockaddr *)&recv_addr: A pointer to a sockaddr structure that will store the address of the sender of the data
        &addr_len: A pointer to the variable that stores the size of the sockaddr structure. This argument is both an input and an output parameter. On input, it specifies the size of the buffer passed in the previous argument. On output, it is set to the actual size of the address structure stored in recv_addr.*/
        // Calculate RTT
        uint32_t recv_timestamp;
        gettimeofday(&current_time, NULL);

        recv_timestamp = current_time.tv_sec * 1000000 + current_time.tv_usec;
        uint16_t recv_seq_num = *((uint16_t *)recv_buf);
        memcpy(&recv_timestamp, recv_buf + sizeof(recv_seq_num), sizeof(recv_timestamp)); 
        //void* memset( void* str, int ch, size_t n);
        /*In this code, recv_timestamp is initialized to the current timestamp at the receiver side when the packet is received using the gettimeofday() function. Then, the recv_buf buffer is used to receive the incoming packets from the server, which is expected to contain the sequence number and the timestamp of the packet that was sent by the client.

        The sequence number is extracted from the recv_buf buffer using the *((uint16_t *)recv_buf) syntax, which casts the buffer to a 16-bit unsigned integer pointer and then dereferences it to obtain the value stored in the first two bytes of the buffer.

        The timestamp value of the received packet is stored after the sequence number in the buffer, so the memcpy() function is used to copy the timestamp value from the buffer into the recv_timestamp variable. The memcpy() function is used because the timestamp is not stored as a single 32-bit integer in the buffer, but rather as a byte array of length 4.

        By subtracting the timestamp value of the received packet from the timestamp value of the packet that was sent, we can calculate the round-trip time (RTT) for the packet, which is then added to the total_rtt variable.
*/
        uint8_t recv_ttl = *((uint8_t *)(recv_buf + sizeof(recv_seq_num) + sizeof(recv_timestamp)));
        double rtt = (double)(recv_timestamp - timestamp) / 1000.0;
        total_rtt += rtt;
        /*The received TTL value is extracted from the recv_buf buffer, which contains the received packet. The value is located after the sequence number and timestamp fields in the packet, so the starting point of the TTL field is calculated by adding the sizes of the sequence number and timestamp fields to the beginning of the buffer. The *((uint8_t *)(recv_buf + sizeof(recv_seq_num) + sizeof(recv_timestamp))) expression is used to cast the starting address of the TTL field to a pointer to a uint8_t type and dereference it to obtain the value.

        The round-trip time is calculated by subtracting the sent timestamp from the received timestamp and converting the result to milliseconds. The rtt variable is used to store the result. The total RTT is accumulated in the total_rtt variable, which is used later to calculate the average RTT.*/

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