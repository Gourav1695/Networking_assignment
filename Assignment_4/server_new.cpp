#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <ServerPort>" << endl;
    exit(EXIT_FAILURE);
}

// Extract command-line argument
int server_port = atoi(argv[1]);

// Create a UDP socket
int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (sockfd < 0) {
    cerr << "Failed to create socket." << endl;
    exit(EXIT_FAILURE);
}

// Set server address
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(server_port);

// Bind socket to server address
if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    cerr << "Failed to bind socket to address." << endl;
    exit(EXIT_FAILURE);
}

while (true) {
    // Receive packet from client
    char recv_buf[1024];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&client_addr, &addr_len);

    // Decrement TTL and forward packet back to client
    uint8_t ttl_val = *((uint8_t*)(recv_buf + sizeof(uint16_t) + sizeof(uint32_t)));
    if (ttl_val > 0) {
        ttl_val--;
        *((uint8_t*)(recv_buf + sizeof(uint16_t) + sizeof(uint32_t))) = ttl_val;
        sendto(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        
    }
    
    /*This code snippet receives a packet from the socket with the file descriptor sockfd using the recvfrom function. The received data is stored in the buffer recv_buf, which has a size of 1024 bytes. The recvfrom function also populates the client_addr structure with the source address and port of the client that sent the packet, and the length of the structure is stored in addr_len.

    The code then extracts the TTL (time-to-live) value from the received packet. If the TTL value is greater than 0, it decrements the TTL value by 1 and updates the TTL field in the packet with the new value. Finally, it forwards the packet back to the client using the sendto function, sending the packet to the address and port specified in the client_addr structure.
*/
}

// Close the socket
close(sockfd);

return 0;
}
