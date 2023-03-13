// 2020CSB010 GOURAV KUMAR SHAW

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define MAX 80 // message size
#define PORT 8080
#define SA struct sockaddr

void *receive_thread_function(void *arg)
{
    int sockfd = *(int *)arg; 

    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, MAX); 

        read(sockfd, buff, sizeof(buff)); // buff is storing the message
        printf("From client: %s\n", buff);
        if (strncmp("exit", buff, 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
    }
}

void *send_thread_function(void *arg)
{
    int sockfd = *(int *)arg; 

    char buff[MAX];
    int n;
    while (1)
    {
        bzero(buff, MAX);
        n = 0;
        while ((buff[n++] = getchar()) != '\n')
            ;

        write(sockfd, buff, sizeof(buff));
        if (strncmp("exit", buff, 4) == 0)
        {
            printf("Server Exit...\n");
            break;
        }
    }
}

int main()
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT);              
    /*
    This line of code binds the created socket to the specified IP address and port number using the bind function. sockfd is the socket file descriptor returned by the socket function. SA is a type defined as a struct sockaddr, which is used for socket addresses. servaddr is an instance of the sockaddr_in structure, which contains the IP address and port number that the server is binding to. The sizeof operator is used to calculate the size of the servaddr structure.

    The bind function returns 0 if the binding is successful, and -1 if an error occurs. If the binding fails, the program will print an error message and exit.
    */
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully bound..\n");
        /*
        This line of code starts the listening process on the server socket created earlier, using the listen function. The listen function puts the server socket in a passive mode, waiting for client connections.

        sockfd is the socket file descriptor returned by the socket function. The second argument, 5, specifies the maximum length to which the queue of pending connections for this socket may grow. This means that the server will be able to handle up to 5 client connections simultaneously.

        The listen function returns 0 if successful, and -1 if an error occurs. If an error occurs, the program will print an error message and exit.
        */
    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
        /*
        This line of code sets the size of the cli structure, which is a sockaddr_in structure containing the client's address and port number.

        The sizeof(cli) returns the size of the cli structure in bytes, which is passed to the accept function as the size of the cli structure. The accept function then populates the cli structure with the client's address and port number information.

        It is important to note that the len variable is passed to the accept function as a pointer, so that the function can update its value with the actual size of the cli structure. This is necessary because the size of the cli structure may vary depending on the type of the address family being used (IPv4 or IPv6).
                */
    len = sizeof(cli);
    /*
    code accepts a connection from a client and creates a new socket for communication with the client. The accept function blocks the server process until a client connects to the server.

    sockfd is the server socket file descriptor returned by the socket function. cli is a pointer to a sockaddr structure that will hold the client's address and port number. len is a pointer to a variable that holds the size of the sockaddr structure.

    The accept function returns a new file descriptor representing the client socket. This file descriptor is used for subsequent communication with the client. If an error occurs, the function returns -1.
        */
    connfd = accept(sockfd, (SA *)&cli, &len);
    if (connfd < 0)
    {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");
    pthread_t receive_tid, send_tid;
    /*
    This line of code declares two variables of type pthread_t: receive_tid and send_tid. pthread_t is a data type in the pthreads library that represents a thread ID.

    The variables are used to store the thread IDs returned by the pthread_create function when it creates two new threads. The first thread is responsible for receiving messages from the client, and the second thread is responsible for sending messages to the client. By storing the thread IDs in variables, the main thread can later use these IDs to join the threads and wait for their completion.
        */
    int *p = malloc(sizeof(int));
    *p = connfd;
    pthread_create(&receive_tid, NULL, receive_thread_function, p);
    pthread_create(&send_tid, NULL, send_thread_function, p);
    pthread_join(receive_tid, NULL);
    pthread_join(send_tid, NULL);
    /*
    The code above is creating two threads, receive_thread_function and send_thread_function, passing the socket file descriptor connfd as an argument, and then waiting for the threads to finish using pthread_join().

    malloc() is used to allocate memory dynamically to store the integer value of connfd, which is then passed as a void pointer to the threads.

    pthread_create() is used to create a new thread. It takes four arguments: a pointer to a pthread_t object to store the ID of the newly created thread, a pthread_attr_t object that specifies various attributes of the thread (e.g., its scheduling policy), a pointer to the function that the thread should run, and a void pointer to any arguments that should be passed to the thread function.

    After both threads have been created, pthread_join() is used to wait for them to finish. pthread_join() takes two arguments: the pthread_t object for the thread to wait for, and a pointer to a location where the exit status of the thread should be stored (if the exit status is not needed, this argument can be NULL).
    */
    close(sockfd);
}
