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

// Structure of the parameters
typedef struct
{
    int key1;            // 4 bytes
    int key2;            // 4 bytes
    char value1[256];    // 255 chars + '\0', it will be tested if strlen(value1) > 256
    int value2;          // 4 bytes
    double value3;       // 8 bytes
} Parameters;