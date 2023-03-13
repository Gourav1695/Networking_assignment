// 2020CSB010 GOURAV KUMAR SHAW

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <signal.h>
#include <unistd.h>

#define PORT 8080
using namespace std;

int client_socket;
void *SendingMessage(void *arg)
{
    pthread_t thread2 = *(pthread_t *)arg;
    while (1)
    {
        char message1[1024];
        cin >> message1;
        if (sizeof(message1) / sizeof(message1[0]) > 0)
        {
            if (send(client_socket, message1, sizeof(message1) / sizeof(message1[0]), 0) == -1)
            {
                cout << "Failed to Send!" << endl;
            }
        }
        if (!strcmp(message1, "exit"))
        {
            if (close(client_socket) == 0)
                cout << "Client Socket Closed!"
                     << endl;
            if (pthread_cancel(thread2) == 0)
                cout << "Thread 2 Cancelled!" << endl;
            pthread_exit(NULL);
        }
    }
    return NULL;
}

void *receivingMessage(void *arg)
{
    pthread_t thread1 = *(pthread_t *)arg;
    while (1)
    {
        char message2[1024];
        int msgSize = recv(client_socket, message2, 1024, 0);
        if (msgSize > 0)
        {
            if (!strcmp(message2, "server down"))
            {
                if (close(client_socket) == 0)
                    cout << "Client Socket Closed!"
                         << endl;
                if (pthread_cancel(thread1) == 0)
                    cout << "Thread 1 Cancelled!" << endl;
                pthread_exit(NULL);
            }
            cout << message2 << endl;
        }
    }
    return NULL;
}

int main()
{
    // create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        cout << "Failed to Create Socket!" << endl;
        exit(EXIT_FAILURE);
    }

    // initialize  sockaddr_in structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        cout << "Error in Connecting to Server!" << endl;
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Client Successfully Connected to Server!" << endl;
    }
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, SendingMessage, &thread2);
    pthread_create(&thread2, NULL, receivingMessage, &thread1);
    pthread_join(thread1, NULL);
    cout << "Thread 1 Closed!" << endl;
    pthread_join(thread2, NULL);
    cout << "Thread 2 Closed!" << endl;
    return 0;
}

