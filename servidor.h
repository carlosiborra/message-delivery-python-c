/*
 * File: servidor.h
 * Authors: 100451339 & 100451170
 */

#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include "LinkedList.h"

// Define true and false as 1 and 0 to avoid using the stdbool.h library
#define true 1    //  Macro to map true to 1
#define false 0   //  Macro to map false to 0

/**
 * @brief Initialise service and destroys all stored tuples.
 * @return 0 if the service was initialised correctly, -1 an error occurred during communication.
 * @note This is a WRITER function.
 */
int list_init();

/**
 * @brief Create a new user in the list with the given parameters.
 * @param ip char*
 * @param port char*
 * @param name char*
 * @param alias char*
 * @param birth char*
 * @return 0 -> Success, 1 -> User already exists, 2 -> Error
 */
uint8_t list_register_user(char *ip, char *port, char *name, char *alias, char *birth);

/**
 * @brief Delete a user from the list with the given alias.
 * @param alias char*
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t list_unregister_user(char *alias);

/**
 * @brief Connect a user with the given alias.
 * @param ip char*
 * @param port char*
 * @param alias char*
 * @return a struct ConnectionResult with error code 0 -> Success, 1 -> User not found, 2 -> User already connected, 3 -> Error
 */
ConnectionResult list_connect_user(char *ip, char *port, char *alias);

/**
 * @brief Disconnect a user with the given alias.
 * @param ip char*
 * @param alias char*
 * @return 0 -> Success, 1 -> User not found, 2 -> User already disconnected, 3 -> Error
 */
uint8_t list_disconnect_user(char *ip, char *alias);

/**
 * @brief Search for all connected users in the list.
 * @param alias char*
 * @return ConnectedUsers struct with error_code: 0 -> Success, 1 -> User not connected, 2 -> User not found, 3 -> Error
 */
ConnectedUsers list_connected_users(char *alias);

/**
 * @brief Send a message from a user to another user.
 * @param sourceAlias char*
 * @param destAlias char*
 * @param message char*
 * @return a struct ReceiverMessage with error code 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
ReceiverMessage list_send_message(char *sourceAlias, char *destAlias, char *message);

/**
 * @brief Get connection status of the user with the given alias.
 *
 * @return a struct ConnectionStatus with error code 0 -> Success (User connected), 1 -> User not found, 2 -> Error
 */
ConnectionStatus list_get_connection_status(char *alias);

/**
 * @brief Display the list.
 * @note This is a WRITER function.
 */
int list_display_user_list();

/**
 * @brief Display the list.
 * @note This is a WRITER function.
 */
int list_display_pending_messages_list(char* alias);

/**
 * @brief Display the list.
 * @note This is a WRITER function.
 */
void request_delete_list();

/**
 * @brief Delete a message from the user list with the given alias and number.
 * 
 * @param alias char*
 * @param num unsigned int
 * @return error code 0 -> Success, 1 -> Error 
 */
uint8_t list_delete_message(char *alias, unsigned int num);

#endif