/*
 * request.h file - structure of the request
 * Authors: 100451339 & 100451170
 */

// Enum to identify the operation to be performed
typedef enum
{
    REGISTER = 0,
    UNREGISTER = 1,
    CONNECT = 2,
    DISCONNECT = 3,
    SEND = 4,
    CONNECTEDUSERS = 5
} OPERATION;

// Array to store the names of the operations
char* OPERATION_NAMES[7] = {"REGISTER", "UNREGISTER", "CONNECT", "DISCONNECT",  "SEND", "CONNECTEDUSERS"};

// Array to store the number of parameters that each operation needs
int OPERATION_PARAMS[7] = {3, 1, 1, 1, 3, 1};

// Structure of the request
typedef struct
{
    int socket; // Socket descriptor
    char operation[256]; // Operation to be performed
} Request;
