// 2020CSB010 GOURAV KUMAR SHAW

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <cstring>
#include <iostream>
#include <unistd.h>

using namespace std;

#define PORT 8080
#define MAX_CLIENTS 5

map<int, char *> m;                 // socket file descriptor and name
map<int, pthread_t> client_threads; // client socket file descriptor and thread
int server_socket;
/*
    m: a map that associates a client socket file descriptor with the name of the client.
    client_threads: a map that associates a client socket file descriptor with the thread that will be used to handle messages to/from that client.
    server_socket: the socket file descriptor that the server will use to listen for connections from clients.
*/

void *sendingMessage(void *arg)
{
    int client_socket = *(int *)arg;
    /*
    Assuming arg was passed as a void pointer to a function that is creating a new thread, this code is extracting the client socket file descriptor from the arg pointer, which was passed to the thread function as an argument. The client socket file descriptor is used to communicate with the client over the network.
    */
    char sender[1024];
    strcpy(sender, m[client_socket]);
    char destination[1024];
    char message[1024];
    while (1)
    {
        bool flag = false;
        /*
        The first argument, client_socket, is a file descriptor for a socket that has been created and connected to a remote host. This socket is used for receiving data from the remote host.

        The second argument, destination, is a pointer to a buffer that will hold the received data. This buffer must be large enough to hold the maximum amount of data that may be received in a single call to recv(). In this case, the buffer size is set to 1024 bytes.

        The third argument, 1024, is the maximum amount of data that recv() will attempt to receive in a single call. This value should not exceed the size of the buffer pointed to by the destination argument.

        The fourth argument, 0, specifies the flags to be used when receiving the data. In this case, no special flags are set.
                */
        int destSize = recv(client_socket, destination, 1024, 0);
        if (destSize > 0)
        {
            int messageSize = recv(client_socket, message, 1024, 0); // message size
            for (auto it : m)
            {
                if (messageSize > 0)
                {
                    if (!strcmp(message, "exit"))
                    {
                        strcpy(m[client_socket], "\0");
                        if (close(client_socket) == 0)
                            cout << "Client Socket Closed!"
                                 << endl;
                    }

                    else if (!strcmp(it.second, destination))
                    {
                        flag = true;
                        if (messageSize > 0)
                        {
                            strcat(message, " from ");
                            strcat(message, m[client_socket]);
                            send(it.first, message, 1024, 0);
                        }
                        break;
                    }
                }
            }
            if (!flag)
            {
                send(client_socket, "Offline!", 1024, 0);
            }
        }
    }
    return NULL;
}
void *close(void *arg)
{
    int server_fd = *(int *)arg;
    char closemsg[1024];
    while (1)
    {
        cout << "Enter (close) to close the server" << endl;
        cin >> closemsg;
        if (!strcmp(closemsg, "close"))
        {
            for (auto i : m)
            {
                write(i.first, "server down", 12);
                if (close(i.first) == 0) // "close(i.first) == 0" is a comparison that evaluates to true if the "close" system call on the file descriptor represented by "i.first" returns 0, indicating that the file descriptor was successfully closed.
                {
                    cout << "Client Socket of  " << i.second << " closed" << endl;
                }
            }
            close(server_fd);
            exit(0);
        }
    }

    return NULL;
}

int main()
{
    // Here Creating Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        cout << "Failed to Create Socket!" << endl;
        exit(EXIT_FAILURE);
    }

    // initializing sockaddr_in structure
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        cout << "Failed to Bind!" << endl;
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // listening
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        cout << "Failed to Listen!" << endl;
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "Server is Listening on Port " << PORT << endl;
    }

    pthread_t exit_thread;
    pthread_create(&exit_thread, NULL, close, &server_socket);

    while (1)
    {
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int client_socket = accept(server_socket, (struct sockaddr *)&client, &len);
        if (client_socket == -1)
        {
            cout << "Client Failed to Connect!!" << endl;
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        char name[1024];
        int nameSize = recv(client_socket, name, 1024, 0);
        if (nameSize > 0)
        {
            m[client_socket] = (char *)malloc(strlen(name) * sizeof(char));
            strcpy(m[client_socket], name);
            cout << "Connection Successful with Client : " << m[client_socket] << " !" << endl;

            pthread_create(&client_threads[client_socket], NULL, &sendingMessage, &client_socket);
        }
        /*
        The "if (nameSize > 0)" condition checks if the size of the client name (in bytes) is greater than zero. If so, it proceeds with creating a new thread to handle the client connection.

        The line "m[client_socket] = (char *)malloc(strlen(name) * sizeof(char));" allocates memory for a new character array in the "m" array at the index "client_socket". The size of the array is determined by the length of the "name" string (excluding the null terminator) multiplied by the size of a single character (in bytes). The "(char *)" typecast is used to convert the void pointer returned by "malloc" into a char pointer that can be assigned to the "m" array.

        The line "strcpy(m[client_socket], name);" copies the contents of the "name" string (including the null terminator) into the newly allocated character array at the index "client_socket" of the "m" array. This allows the server to keep track of the name of the client associated with each socket.

        The line "cout << "Connection Successful with Client : " << m[client_socket] << " !" << endl;" outputs a message to the console indicating that a connection has been established with the client associated with the current socket.

        The line "pthread_create(&client_threads[client_socket], NULL, &sendingMessage, &client_socket);" creates a new thread using the "pthread_create" function. The thread is created with the "sendingMessage" function as the entry point and the current socket index as the argument. This allows the new thread to handle messages sent by the client on this socket while the main thread continues to listen for new connections on other sockets.
        */
    }
    return 0;
}
