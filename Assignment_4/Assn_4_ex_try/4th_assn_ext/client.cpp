// 2020CSB010 GOURAV KUMAR SHAW

#include <bits/stdc++.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXSIZE 1600
using namespace std;
// Driver code
int main(int argc, char *argv[])
{
    // defining the payload

    unordered_map<int, int> m;
    FILE *fp1;
    int k = 100;
    
    vector<int> vec1;
    vector<int> vec2;
    for (int i = 0; i < 10; i++)
    {
        m[i] = k;
        k += 100;
    }
    if (argc != 6)
    {
        std::cerr << "Use the following arguments: " << argv[0] << " [ip_address] [port_number] [TTL] [NumPackets] [Filename.csv]\n";
        return 1;
    }
    srand(time(NULL));
    const int PORT = atoi(argv[2]);
    int sockfd;
    char filename[10];
    memcpy(filename, argv[5], sizeof(argv[5]));
    printf("Filename is %s\n", filename);

    struct sockaddr_in servaddr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

   
    socklen_t len;
    struct timeval timestamp;
    struct timeval timestamp1;
    
    int numPacket = atoi(argv[4]);
    
    fp1 = fopen(filename, "w");
    fprintf(fp1, "%s,", "sl.no");
    fprintf(fp1, "%s,", "payload_length");
    fprintf(fp1, "%s \n", "Cumulative_RTT");
    for (int i = 0; i < numPacket; i++)
    { 
        char buffer[MAXSIZE];
        memset(buffer, '\0', MAXSIZE);
        int payload_length = m[i % 10];
        printf("Payload length made as:%d\n", payload_length);
        int ttl = atoi(argv[3]);
        if (ttl % 2 != 0)
            ttl = ttl - 1;
        
        cout <<"ttl as: "<< ttl << endl;
       // making payload with random alphabets
        char payload[payload_length + 1];
        for (int j = 0; j < payload_length-10; j++)
        {
            payload[j] = 'a' + rand() % 26;
        }
        payload[payload_length] = '\0';

        gettimeofday(&timestamp, NULL);
        long int microseconds = timestamp.tv_usec;
        if (i <= 9)
        {
            buffer[0] = '0';
            snprintf(buffer + 1, MAXSIZE, "%d%ld", i, microseconds); // i and microseconds variables, formatting them into a string using the specified format string, and then writing that string to the buffer starting at the second position of the buffer
        }
        else
            snprintf(buffer, MAXSIZE, "%d%ld", i, microseconds);

        buffer[8] = ttl;
        buffer[9] = payload_length;

 
        strcat(buffer, payload);
   
        // strcat(buffer, filename);
            //  cout<<"size: "<<strlen(buffer);
        int n;
        while (ttl)
        {
            sendto(sockfd, (const char *)buffer, strlen(buffer),
                   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
                   sizeof(servaddr));
            std::cout << "Client:  "
                      << "Sequence Number:" << buffer[0] << buffer[1] << std::endl
                      << "time:" << microseconds << std::endl
                      << "ttl:" << ttl << std::endl;
            std::cout << "above message sent\n";
            n = recvfrom(sockfd, (char *)buffer, MAXSIZE,
                         MSG_WAITALL, (struct sockaddr *)&servaddr,
                         &len);
            ttl = buffer[8];
            ttl--;
            buffer[8] = ttl;
        }

        gettimeofday(&timestamp1, NULL);
        long int microsecond = timestamp1.tv_usec;// gives the time in microseconds
        ttl = buffer[8];
        buffer[n] = '\0';
       

        int Cumulative_RTT = microsecond - microseconds;
        cout << "Starting time is:" << microseconds <<" microseconds"<< endl;
        cout << "ending time is:" << microsecond <<" microseconds"<< endl;
        cout << "This is the Cumulative_RTT-->" << Cumulative_RTT <<" microseconds"<< endl;
     
        vec1.push_back(payload_length);
        vec2.push_back(Cumulative_RTT);

        // char temp1[5];
        // char temp2[5];
        
        fprintf(fp1, "%d,", i);
        fprintf(fp1, "%d,", payload_length);
        fprintf(fp1, "%d \n", Cumulative_RTT);
    }
    for (auto it : vec1)
    {
        cout << it << " ";
    }
    cout << endl; 
    for (auto it1 : vec2)
    {
        cout << it1 << " ";
    }

    int err = close(sockfd);
    return 0;
}