#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// Structure for the pending messages
typedef struct MessageEntry
{
    unsigned int num;           // Message number in the list of pending messages
    unsigned int msgId;         // Message ID sent by the sending user
    char sourceAlias[256];      // Alias of the sending user: 255 characters + '\0'
    char message[256];         // Message: 255 characters + '\0'
    struct MessageEntry *next;  // Pointer to the next message in the list
} MessageEntry;

// Implement a linked list of MessageEntry
typedef struct
{
    MessageEntry *head;        // Pointer to the first message in the list
    int size;                  // Number of pending messages
} MessageList;

typedef struct UserEntry
{
    char ip[16];                    // IP address of the user "255.255.255.255" + '\0'
    char port[6];                   // Port of the user "65535" + '\0'
    char name[256];                 // Name of the user: 255 characters + '\0'
    char alias[256];                // Alias of the user: 255 characters + '\0' <- IDENTIFIER
    char birth[11];                 // Birth of the user: "DD/MM/AAAA" + '\0'
    unsigned int messageId;         // Last ID of the message sent by the user
    uint8_t status;                 // Status of the user: 0 -> Disconnected, 1 -> Connected
    MessageList *pendingMessages;   // List of pending messages
    struct UserEntry *next;         // Pointer to the next user in the list
} UserEntry;

// Implement a linked list of UserEntry
typedef struct
{
    UserEntry *head;
    int size;
} UserList;

typedef struct
{
    char* alias[256];               // Array of aliases of the connected users
    unsigned int size;              // Number of connected users
    uint8_t error_code;             // Error code: 0 -> Success, 1 -> User not connected, 2 -> User not found, 3 -> Error
} ConnectedUsers;

/**
 * @brief Search for a user with the given alias in the list.
 * @return NULL if the alias does not exist in the list. Otherwise, return a pointer to the user entry.
 */
UserEntry *search(UserList *list, char *alias);

/**
 * @brief Create a new user in the list with the given parameters.
 * 1. Validate the parameters (ip and port)
 * 2. Search for the user with the given alias in the list. If it exists, return 1.
 * 3. Create a new user entry with the given parameters.
 * 4. Add the user entry to the list.
 * @return 0 -> Success, 1 -> User already exists, 2 -> Error
 */
uint8_t register_user(UserList *list, char *ip, char *port, char *name, char *alias, char *birth);

/**
 * @brief Delete a user from the list with the given alias.
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. Delete all the pending messages of the user
 * 3. Delete the pointer to the pendingMessages list.
 * 4. Delete the user from the list.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t unregister_user(UserList *list, char *alias);

/**
 * @brief Connect a user with the given alias. 
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. If the user is already connected, return 2.
 * 3. If the user is not connected and exists, set the status to 1 (connected).
 * 4. Otherwise, return 3.
 * @return 0 -> Success, 1 -> User not found, 2 -> User already connected, 3 -> Error
 */
uint8_t connect_user(UserList *list, char* ip, char *port, char *alias);

/**
 * @brief Disconnect a user with the given alias.
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. If the user is not connected, return 2.
 * 3. If the user is connected and exists, set the status to 0 (disconnected).
 * 4. Otherwise, return 3.
 * @return 0 -> Success, 1 -> User not found, 2 -> User already disconnected, 3 -> Error
 */
uint8_t disconnect_user(UserList *list, char* ip,  char *alias);

/**
 * @brief Search for all connected users in the list.
 * 1. Search for the user with the given alias in the list. If does not exist, return 2.
 * 2. If user is not connected, return 1.
 * 3. If user is connected, return 0 and the list of connected users with the total number of connected users.
 * @return ConnectedUsers struct with error_code: 0 -> Success, 1 -> User not connected, 2 -> User not found, 3 -> Error
 */
ConnectedUsers connected_users(UserList *list, char *alias);

/**
 * @brief Send a message from a user to another user. 
 * 1. Validate the message length.
 * 2. Search for the source user in the list. If it does not exist, return 2.
 * 3. If the source user is not connected, return 3.
 * 4. Search for the destination user in the list. If it does not exist, return 1.
 * 5. Obtain the last message ID of the source user and increment it by 1 (taking into account the wrap-around to 0).
 * 6.a. If the destination user is connected, send the message to the destination user.
 * 6.b. If the destination user is not connected, store the message in the pending messages list of the destination user and local variable <stored> to 1.
 * @return 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
uint8_t send_message(UserList *list, char *sourceAlias, char *destAlias, char *message);

/**
 * @brief Initialise service and destroys all stored users and pending messages of those users.
 * @return 0 if the service was initialised correctly, -1 an error occurred during communication.
 */
uint8_t init(UserList *list);

/**
 * @brief Delete a user entry.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_user_entry(UserEntry* user);

/**
 * @brief Delete the user list.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_user_list(UserList *list);

/**
 * @brief Delete a message entry.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_message_entry(MessageEntry* message);

/**
 * @brief Delete all pending messages of the user with the given alias.
 * 1. Search for the user with the given alias in the list. If it does not exist, return 1;
 * 2. Delete all the pending messages of the user.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t delete_pending_messages(UserList *list, char *alias);

/**
 * @brief Delete the message list.
 */
void delete_pending_message_list(MessageList *list);

/**
 * @brief Create a new message in the list with the given parameters.
 * 1. Search for the destination user in the list. If it does not exist, return NULL.
 * 2. Create a new message entry with the given parameters.
 * 3. Add the message entry to the list of pending messages of the destination user.
 * @return 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
uint8_t add_pending_message(UserEntry *dest_user, char *sourceAlias, unsigned int msgId, char *message);

/**
 * @brief Display the list of users.
 */
void display_users(UserList *list);

/**
 * @brief Display the list of pending messages of the user with the given alias.
 * 1. Search for the user with the given alias in the list. If it does not exist, return 1;
 * 2. Display all the pending messages of the user.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t display_pending_messages(UserList *list, char *alias);

/**
 * @brief Create a new linked list of users.
 */
UserList *create_user_list();

/**
 * @brief Create a new linked list of pending messages.
 */
MessageList *create_message_list();

#endif
