/*
 * File: LinkedList.c
 * Authors: 100451339 & 100451170
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
#include <stdint.h>

#include "LinkedList.h"

#define localhost "127.0.0.1"
#define UINT_MAX 4294967295 // Maximum value for an unsigned int

/**
 * @brief Search for a user with the given alias in the list.
 * @return NULL if the alias does not exist in the list. Otherwise, return a pointer to the user entry.
 */
UserEntry *search(UserList *list, char *alias) {
  UserEntry *current = list->head;
    while (current != NULL) {
        if (strcmp(current->alias, alias) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

uint8_t validate_ip_port(char* ip, char *port) {
    // Validate port
    char *end;
    int port_int = strtol(port, &end, 10);
    if (*end != '\0') {
        return 1;
    }
    if (port_int <= 1024 || port_int > 65535) {
        return 1;
    }

    // Validate IP
    if (strlen(ip) > 15) {
        return 1;
    }

    int num[4]; // For storing the 4 octets of the IP address
    int count;  // For checking the number of parsed elements

    // Parse the input string into the num array, check if there are exactly 4 octets, and check if there is any extra content
    count = sscanf(ip, "%d.%d.%d.%d", &num[0], &num[1], &num[2], &num[3]);

    if (count != 4) {
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        // Check if each number is within the valid range
        if (num[i] < 0 || num[i] > 255) {
            return 1;
        }
    }


    return 0;
}

/**
 * @brief Create a new user in the list with the given parameters.
 * 1. Validate the parameters (ip and port)
 * 2. Search for the user with the given alias in the list. If it exists, return 1.
 * 3. Create a new user entry with the given parameters.
 * 4. Add the user entry to the list.
 * @return 0 -> Success, 1 -> User already exists, 2 -> Error
 */
uint8_t register_user(UserList *list, char *ip, char *port, char *name, char *alias, char *birth) {
    // Validate ip and port
    if (validate_ip_port(ip, port)) {
        return 2;
    }

    // Check if user already exists
    UserEntry *existing = search(list, alias);
    if (existing != NULL) {
        return 1;
    }

    // Create a new user entry
    UserEntry *new_user = (UserEntry *)malloc(sizeof(UserEntry));
    if (new_user == NULL) {
        return 2;
    }

    strncpy(new_user->ip, ip, 16);
    strncpy(new_user->port, port, 6);
    strncpy(new_user->name, name, 256);
    strncpy(new_user->alias, alias, 256);
    strncpy(new_user->birth, birth, 11);
    new_user->messageId = 0;                                // Initial message ID is 0
    new_user->status = 0;                                   // Initial status is disconnected (0)
    new_user->pendingMessages = create_message_list();      // Create a new list of pending messages
    new_user->next = NULL;                                  // User is at the end of the list, so next is NULL

    // Add the user entry to the list
    if (list->head == NULL) {
        list->head = new_user;
    } else {
        UserEntry *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_user;
    }

    list->size++;
    return 0;
}

/**
 * @brief Delete a user from the list with the given alias.
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. Delete all the pending messages of the user
 * 3. Delete the pointer to the pendingMessages list.
 * 4. Delete the user from the list.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t unregister_user(UserList *list, char *alias) {
    UserEntry *previous = NULL;
    UserEntry *current = list->head;

    while (current != NULL) {
        if (strcmp(current->alias, alias) == 0) {
            // Delete all pending messages
            delete_pending_message_list(current->pendingMessages);

            // Delete the user from the list
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }

            list->size--;
            delete_user_entry(current);
            return 0;
        }
        previous = current;
        current = current->next;
    }

    return 1;
}

/**
 * @brief Connect a user with the given alias.
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. If the user is already connected, return 2.
 * 3. If the user is not connected and exists, set the status to 1 (connected).
 * 4. Otherwise, return 3.
 * @return 0 -> Success, 1 -> User not found, 2 -> User already connected, 3 -> Error
 */
uint8_t connect_user(UserList *list, char* ip, char* port, char* alias) {
    UserEntry *user = search(list, alias);
    if (user == NULL) {
        return 1;
    }
    if (user->status == 1) {
        return 2;
    }
    strncpy(user->ip, ip, 16);              // Update IP
    strncpy(user->port, port, 6);           // Update port
    user->status = 1;                       // Set status to connected
    return 0;
}

/**
 * @brief Disconnect a user with the given alias.
 * 1. Search for the user with the given alias in the list. If does not exist, return 1.
 * 2. If the user is not connected, return 2.
 * 3. If the user is connected and exists, set the status to 0 (disconnected).
 * 4. Otherwise, return 3.
 * @return 0 -> Success, 1 -> User not found, 2 -> User already disconnected, 3 -> Error
 */
uint8_t disconnect_user(UserList *list, char* ip, char *alias) {
    UserEntry *user = search(list, alias);
    if (user == NULL) {
        return 1;
    }
    if (user->status == 0) {
        return 2;
    }
    if (strcmp(user->ip, ip) != 0) {
        return 3;
    }
    user->status = 0;
    return 0;
}

/**
 * @brief Search for all connected users in the list.
 * 1. Search for the user with the given alias in the list. If does not exist, return 2.
 * 2. If user is not connected, return 1.
 * 3. If user is connected, return 0 and the list of connected users with the total number of connected users.
 * @return 0 -> Success, 1 -> User not connected, 2 -> User not found, 3 -> Error
 */
ConnectedUsers connected_users(UserList *list, char *alias) {
    UserEntry *user = search(list, alias);
    ConnectedUsers result;
    result.size = 0;
    result.error_code = 0;

    if (user == NULL) {
        result.error_code = 2;
        return result;
    }
    if (user->status == 0) {
        result.error_code = 1;
        return result;
    }

    UserEntry *current = list->head;
    while (current != NULL) {
        if (current->status == 1) {
            result.alias[result.size] = current->alias;
            result.alias[result.size][255] = '\0'; // Make sure the string is null-terminated
            result.size++;
        }
        current = current->next;
    }
    return result;
}

/**
 * @brief Send a message from a user to another user.
 * 1. Validate the message length.
 * 2. Search for the source user in the list. If it does not exist, return 2.
 * 3. If the source user is not connected, return 3.
 * 4. Search for the destination user in the list. If it does not exist, return 1.
 * 5. Obtain the last message ID of the source user and increment it by 1 (taking into account the wrap-around to 0).
 * 6.a. If the destination user is connected, send the message to the destination user.
 * 6.b. If the destination user is not connected, store the message in the pending messages list of the destination user and local variable <stored> to 1.
 * @return a ReceiverMessage struct with error_code 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
ReceiverMessage send_message(UserList *list, char *sourceAlias, char *destAlias, char *message) {
    ReceiverMessage result;
    strcpy(result.ip, "");
    strcpy(result.port, "");
    result.error_code = 0;
    result.msgId = 0;
    
    if (strlen(message) > 255) {
        result.error_code = 2;
        return result;
    }

    UserEntry *source_user = search(list, sourceAlias);
    if (source_user == NULL) {
        result.error_code = 2;
        return result;
    }
    if (source_user->status == 0) {
        result.error_code = 2;
        return result;
    }

    UserEntry *dest_user = search(list, destAlias);
    if (dest_user == NULL) {
        result.error_code = 2;
        return result;
    }

    source_user->messageId = (source_user->messageId + 1) % UINT_MAX;

    if (dest_user->status == 1) {
        // Send the message to the destination user
        strcpy(result.ip, dest_user->ip);
        strcpy(result.port, dest_user->port);
        result.msgId = source_user->messageId;
    } else {
        add_pending_message(dest_user, sourceAlias, source_user->messageId, message);
    }

    return result;
}

/**
 * @brief Initialise service and destroys all stored users and pending messages of those users.
 * @return 0 if the service was initialised correctly, -1 an error occurred during communication.
 */
uint8_t init(UserList *list) {
    if (list == NULL) {
        return -1;
    }
    delete_user_list(list);
    list->head = NULL;
    list->size = 0;
    return 0;
}

/**
 * @brief Delete a user entry.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_user_entry(UserEntry *user) {
    if (user == NULL) {
        return 1;
    }
    // delete all the pending messages of the user
    delete_pending_message_list(user->pendingMessages);
    free(user->pendingMessages);
    free(user);
    return 0;
}

/**
 * @brief Delete the user list.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_user_list(UserList *list) {
    UserEntry *current = list->head;
    while (current != NULL) {
        UserEntry *next = current->next;
        delete_user_entry(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
    return 0;
}

/**
 * @brief Delete a message entry.
 * @return 0 -> Success, 1 -> Error
 */
uint8_t delete_message_entry(MessageEntry* message) {
    if (message == NULL) {
        return 1;
    }
    free(message);
    return 0;
}

/**
 * @brief Delete all pending messages of the user with the given alias.
 * 1. Search for the user with the given alias in the list. If it does not exist, return 1;
 * 2. Delete all the pending messages of the user.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t delete_pending_messages(UserList *list, char *alias) {
    UserEntry *user = search(list, alias);
    if (user == NULL) {
        return 1;
    }
    delete_pending_message_list(user->pendingMessages);
    return 0;
}

/**
 * @brief Delete the message list.
 */
void delete_pending_message_list(MessageList *list) {
    MessageEntry *current = list->head;
    while (current != NULL) {
        MessageEntry *next = current->next;
        delete_message_entry(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}

/**
 * @brief Create a new message in the list with the given parameters.
 * 1. Search for the destination user in the list. If it does not exist, return NULL.
 * 2. Create a new message entry with the given parameters.
 * 3. Add the message entry to the list of pending messages of the destination user.
 * @return 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
uint8_t add_pending_message(UserEntry *dest_user, char *sourceAlias, unsigned int msgId, char *message) {
    MessageEntry *new_message = (MessageEntry *)malloc(sizeof(MessageEntry));
    if (new_message == NULL) {
        return 1;
    }

    new_message->num = dest_user->pendingMessages->size;
    new_message->msgId = msgId;
    strncpy(new_message->sourceAlias, sourceAlias, 255);
    new_message->sourceAlias[255] = '\0';
    strncpy(new_message->message, message, 255);
    new_message->message[255] = '\0';
    new_message->next = NULL;

    if (dest_user->pendingMessages->head == NULL) {
        dest_user->pendingMessages->head = new_message;
    } else {
        MessageEntry *current = dest_user->pendingMessages->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_message;
    }

    dest_user->pendingMessages->size++;
    return 0;
}

/**
 * @brief Display the list of users.
 */
void display_users(UserList *list) {
    setlocale(LC_ALL, "");              // Enable Unicode support in printf
    UserEntry *current = list->head;
    while (current != NULL) {
        printf("👤 Alias: %s, 🌐 IP: %s, 🚪 Port: %s, 📛 Name: %s, 🎂 Birth: %s, 🔌 Status: %s\n",
               current->alias, current->ip, current->port, current->name, current->birth,
               current->status ? "Connected" : "Disconnected");
        current = current->next;
    }
}
/**
 * @brief Display the list of pending messages of the user with the given alias.
 * 1. Search for the user with the given alias in the list. If it does not exist, return 1;
 * 2. Display all the pending messages of the user.
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t display_pending_messages(UserList *list, char *alias) {
    setlocale(LC_ALL, "");              // Enable Unicode support in printf
    UserEntry *user = search(list, alias);
    if (user == NULL) {
        return 1;
    }

    MessageEntry *current = user->pendingMessages->head;
    while (current != NULL) {
        printf("✉️ Message %u from %s: %s\n", current->msgId, current->sourceAlias, current->message);
        current = current->next;
    }
    return 0;
}

/**
 * @brief Create a new linked list of users.
 */
UserList *create_user_list() {
    UserList *list = (UserList *)malloc(sizeof(UserList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

/**
 * @brief Create a new linked list of pending messages.
 */
MessageList *create_message_list() {
    MessageList *list = (MessageList *)malloc(sizeof(MessageList));
    if (list == NULL) {
        return NULL;
    }
    list->head = NULL;
    list->size = 0;
    return list;
}

// void print_register_result(uint8_t register_result) {
//     switch (register_result) {
//         case 0:
//             printf("OKAY: User registered\n");
//             break;
//         case 1:
//             printf("ERROR: User already exists\n");
//             break;
//         case 2:
//             printf("ERROR: Error registering user\n");
//             break;
//     }
// }

// void print_connected_users(ConnectedUsers connected_users_result) {
//     printf("\nConnected users (size: %d, error code: %d):\n", connected_users_result.size, connected_users_result.error_code);
//     for (unsigned int i = 0; i < connected_users_result.size; i++) {
//         printf("\t👤 Alias: %s\n", connected_users_result.alias[i]);
//     }
//     printf("\n");
// }

// int main(void)
// {
//     register_user(user_list, "127.0.0.1", "3000", "Carlos Iborra", "carlitos", "01/01/2000");
//     register_user(user_list, "127.0.0.1", "3000", "Rafael Constasti", "el veneco", "01/01/2002");
//     connect_user(user_list, "127.0.0.1", "3000", "el veneco");
//     // print_register_result(register_result);
//     unregister_user(user_list, "el veneco");
//     register_user(user_list, "127.0.0.1", "3000", "Rafael Constasti", "el veneco", "01/01/2002");

//     connect_user(user_list, "127.0.0.1", "3000", "carlitos");
//     disconnect_user(user_list, "127.0.0.1", "carlitos");
//     connect_user(user_list, "127.0.0.1", "3000", "carlitos");
//     // printf("connect user value: %d", connect_user(user_list, "127.0.0.1", "3000", "carlitos"));

//     register_user(user_list, "127.0.0.1", "3000", "Dibox", "diboxo1", "01/06/2002");
//     connect_user(user_list, "127.0.0.1", "3000", "diboxo1");

//     ConnectedUsers connected_users_result = connected_users(user_list, "carlitos");
//     print_connected_users(connected_users_result);

//     printf("Users after registering carlitos:\n");
//     display_users(user_list);

//     printf("Pending messages of el veneco:\n");
//     send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa cruck");
//     send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa maquina");
//     send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa fiera");
//     send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa mastodonte");
//     send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa campeon");
//     display_pending_messages(user_list, "el veneco");
// }
