/*
 * File: proxy.c
 * Authors: 100451339 & 100451170
 */

// ! Libraries declaration
#include <fcntl.h>      /* For O_* constants */
#include <sys/stat.h>   /* For mode constants */
#include <sys/socket.h> /* For socket(), connect(), send(), and recv() */
#include <netinet/in.h> /* For sockaddr_in and inet_addr() */
#include <arpa/inet.h>  /* For inet_addr() */
#include <stdio.h>      /* For printf */
#include <stdlib.h>     /* For exit */
#include <signal.h>     /* For signal */
#include <string.h>     /* For strlen, strcpy, sprintf */
#include <unistd.h>     /* For getpid */

#include "request.h"  /* For request struct */
#include "servidor.h" /* For server functions */
#include "lines.h"    /* For reading the lines send from a socket */

#define MAX_LINE 256

// ! Mutex & Condition variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int busy = true;

// ! Signal handler
// Using a signal handler to stop the server, forced to declare and use signum to avoid warnings
void stopServer(int signum)
{
    printf("\n\nClosing the server and deleting the users list...\n\n");

    request_delete_list();

    exit(signum);
}

/**
 * @brief Create the socket
 *
 * @param port
 * @return int
 */
int create_socket(int port)
{
    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd == -1)
    {
        perror("Error creating the socket");
        exit(1);
    }

    // ! Socket options
    int optval = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) == -1)
    {
        perror("Error setting socket options");
        exit(1);
    }

    // ! Bind the socket
    struct sockaddr_in server_addr = {0};
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding the socket");
        exit(1);
    }

    // ! Listen for connections
    if (listen(sd, SOMAXCONN) == -1)
    {
        perror("Error listening for connections");
        exit(1);
    }

    return sd;
}

char *read_string(int sd)
{
    char *string = malloc(MAX_LINE);

    ssize_t bytes_read = readLine(sd, string, MAX_LINE);
    if (bytes_read == -1)
    {
        perror("Error reading the string");
        // TODO: Hacer un free del string (cuidao con valgrind)
        return NULL;
    }

    return string;
}

void send_int(int sd, int int_value)
{
    if (send(sd, &int_value, sizeof(int), 0) == -1)
    {
        printf("Error sending integer value to the server\n");
        exit(1);
    }
}

void send_string(int sd, char *string)
{
    int len = strlen(string) + 1;

    if ((sendMessage(sd, string, len)) == -1)
    {
        printf("Error sending string to the server\n");
        exit(1);
    }
}

// Parameters get_parameters(int client_sd, int operation_code) {
//     int num_parameters = OPERATION_PARAMS[operation_code];

//     Parameters parameters;

//     if(strcmp(OPERATION_NAMES[operation_code], "copy_key") == 0) {
//         parameters.key1 = read_int(client_sd);
//         parameters.key2 = read_int(client_sd);
//     } else {
//         for (int i = 0; i < num_parameters; i++)
//         {
//             switch (i)
//             {
//             case 0:
//                 parameters.key1 = read_int(client_sd);
//                 break;
//             case 1:
//                 readLine(client_sd, parameters.value1, MAX_LINE);
//                 break;
//             case 2:
//                 parameters.value2 = read_int(client_sd);
//                 break;
//         }
//     }

//     return parameters;
// }

int8_t get_operation_code(char *operation_code_str)
{
    // * Get the operation code (int)
    int8_t operation_code_int = -1;
    for (int i = 0; i < 7; i++)
    {
        if (strcmp(operation_code_str, OPERATION_NAMES[i]) == 0)
        {
            operation_code_int = i;
            break;
        }
    }

    if (operation_code_int == -1)
    {
        printf("Error: Invalid operation code\n");
        exit(1);
    }

    return operation_code_int;
}

/**
 * @brief Send the response to the client
 * @param socket (socket descriptor of the client)
 * @param error_code
 */
void send_error_code(int socket, char error_code)
{
    sendMessage(socket, &error_code, sizeof(char));
}

/**
 * @brief Deal with the request
 *
 * @param client_request
 */
void deal_with_request(Request *client_request)
{
    // Parameters parameters = {0};

    char operation_code_copy[256] = "";
    int client_sd = 0;
    char client_IP[16] = "";
    int client_port = 0;
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr);

    // * Lock the mutex on the process of request copying
    pthread_mutex_lock(&mutex);
    strcpy(operation_code_copy, client_request->operation);
    client_sd = client_request->socket;

    // get the client IP and port
    getpeername(client_sd, (struct sockaddr *)&client_addr, &client_addr_len);
    strcpy(client_IP, inet_ntoa(client_addr.sin_addr));
    client_port = ntohs(client_addr.sin_port);

    // print the client IP and port
    printf("IP: %s, Port: %d\n", client_IP, client_port);

    busy = false;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // * Get the operation code (int)
    int8_t operation_code_int = get_operation_code(operation_code_copy);

    // * Get the number of parameters
    // int num_parameters = OPERATION_PARAMS[operation_code_int];

    char client_port_str[6];
    sprintf(client_port_str, "%d", client_port);

    char port[6];
    char name[256];  // Name of the user: 255 characters + '\0'
    char alias[256]; // Alias of the user: 255 characters + '\0' <- IDENTIFIER
    char birth[11];  // Birth of the user: "DD/MM/AAAA" + '\0'

    uint8_t error_code;
    switch (operation_code_int)
    {
        case REGISTER:
            // * Read the parameters
            strcpy(name, read_string(client_sd));
            strcpy(alias, read_string(client_sd));
            strcpy(birth, read_string(client_sd));

            // * Register the user
            error_code = list_register_user(client_IP, client_port_str, name, alias, birth);
            list_display_user_list();

            // * Send the error code to the client
            send_error_code(client_sd, error_code);

            break;

        case UNREGISTER:

            // * Read the parameters
            strcpy(alias, read_string(client_sd));
            
            // * Unregister the user
            error_code = list_unregister_user(alias);
            list_display_user_list();

            // * Send the error code to the client
            send_error_code(client_sd, error_code);

            break;

        case CONNECT:

            // * Read the parameters
            strcpy(alias, read_string(client_sd));
            strcpy(port, read_string(client_sd));


            // * Connect the user
            error_code = list_connect_user(client_IP, port, alias);
            list_display_user_list();

            // * Send the error code to the client
            send_error_code(client_sd, error_code);

            break;

        case DISCONNECT:

            // * Read the parameters
            strcpy(alias, read_string(client_sd));

            // * Disconnect the user
            error_code = list_disconnect_user(client_IP, alias);
            list_display_user_list();

            // * Send the error code to the client
            send_error_code(client_sd, error_code);

            break;

        case CONNECTEDUSERS: 
            // * Read the parameters
            strcpy(alias, read_string(client_sd));
 
            // * Get the list of connected users
            ConnectedUsers connUsers = list_connected_users(alias);
            list_display_user_list();

            // * Send the list of connected users to the client
            send_error_code(client_sd, connUsers.error_code);

            // * Send the number of connected users to the client
            char connUserSize[7];
            sprintf(connUserSize, "%u", connUsers.size);
            send_string(client_sd, connUserSize);

            // * Send the list of connected users to the client
            for (unsigned int i = 0; i < connUsers.size; i++) {
                send_string(client_sd, connUsers.alias[i]);
            }

            break;
    }

    // close the socket
    close(client_sd);
}

/**
 * @brief Get the port number from the user
 *
 * @param argc
 * @param argv
 * @return int
 */
int process_port(int argc, char *argv[])
{
    char *end;

    if (argc != 3)
    {
        printf("Usage: %s -p <port>\n", argv[0]);
        exit(1);
    }

    // Now we are sure that the user has only entered the port
    // -> Let's check if it's an integer and then convert it to int
    int port = (int)strtol(argv[2], &end, 10);

    if (*end != '\0')
    {
        printf("Invalid port number format: %s\n", argv[2]);
        exit(1);
    }

    if (port < 1024 || port > 65535)
    {
        printf("Invalid port number: %d\n", port);
        exit(1);
    }

    return port;
}

int main(int argc, char *argv[])
{
    int port = process_port(argc, argv);
    // printf("Port: %d\n", port);
    int sd = create_socket(port);

    // Register the signal handler
    signal(SIGINT, stopServer);

    // If signal is received, stop the server
    signal(SIGINT, stopServer);

    // ! Thread attributes
    pthread_attr_t attr;                                         // Thread attributes
    pthread_attr_init(&attr);                                    // Initialize the attribute
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); // Set the attribute to detached

    // ! Mutex & Condition variables
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    pthread_cond_init(&cond, NULL);   // Initialize the condition variable

    // of messages sent/set of messages received and so we dont have to force break the loop
    while (1)
    {
        // * Open the client socket
        Request client_request = {0};
        struct sockaddr_in client_addr = {0};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sd = accept(sd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_sd == -1)
        {
            perror("Error opening the client socket");
            exit(1);
        }

        // * Create the request
        client_request.socket = client_sd;
        strcpy(client_request.operation, read_string(client_sd));

        printf("📧 Operation -> \"%s\"\n", client_request.operation);

        // * Print the request
        // ! We create a thread for each request and execute the function deal_with_request
        pthread_t thread; // create threads to handle the requests as they come in

        pthread_create(&thread, &attr, (void *)deal_with_request, (void *)&client_request);

        // ! Wait for the thread to finish (child copies the descriptor)
        pthread_mutex_lock(&mutex); // Lock the mutex
        while (busy)
        {
            pthread_cond_wait(&cond, &mutex); // Wait for the condition variable
        }
        busy = true;                  // Set the thread as busy
        pthread_mutex_unlock(&mutex); // Unlock the mutex
    }

    // ! Destroy the mutex and the condition variable
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    close(sd);

    return 0;
}
