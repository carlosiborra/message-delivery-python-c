/*
 * servidor.c
 * Authors: 100451339 & 100451170
 */

#include "servidor.h"

// We'll use semaphores to control the access as readers/writers
#include <semaphore.h>
#include <pthread.h>

UserList *user_list = NULL;

// The wait() and post() semaphores are atomic -> do not assure order

sem_t writer_sem;                                       // semaphores for readers and writers
pthread_mutex_t reader_mut = PTHREAD_MUTEX_INITIALIZER; // mutex for the reader_count variable and its initialization
int reader_count = 0;                                   // number of readers reading
uint8_t is_semaphore_initialized = false;                  // semaphore initialization flag
uint8_t is_list_created = false;                           // linked list creation flag

void init_sem()
{
    // Initialize the semaphore
    pthread_mutex_lock(&reader_mut);
    if (!is_list_created)
    {
        user_list = create_user_list();
        is_list_created = true;
    }

    if (!is_semaphore_initialized)
    {
        sem_init(&writer_sem, 0, 1);
        is_semaphore_initialized = true;
    }
    pthread_mutex_unlock(&reader_mut);
}

/**
 * @brief Initialise service and destroys all stored tuples.
 * @return 0 if the service was initialised correctly, -1 an error occurred during communication.
 * @note This is a WRITER function.
 */
int list_init()
{
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);

    // initialize linked list
    int error_code = init(user_list);

    // Writer releases the write semaphore
    sem_post(&writer_sem);

    return error_code;
}

/**
 * @brief Create a new user in the list with the given parameters.
 * @param list UserList
 * @param ip char*
 * @param port char*
 * @param name char*
 * @param alias char*
 * @param birth char*
 * @return 0 -> Success, 1 -> User already exists, 2 -> Error
 */
uint8_t list_register_user(UserList *list, char *ip, char *port, char *name, char *alias, char *birth) {
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);

    // Create user in the linked list
    int error_code = register_user(list, ip, port, name, alias, birth);

    // Writer releases the write semaphore
    sem_post(&writer_sem);

    return error_code;
}

/**
 * @brief Delete a user from the list with the given alias.
 * @param list UserList
 * @param alias char*
 * @return 0 -> Success, 1 -> User not found, 2 -> Error
 */
uint8_t list_unregister_user(UserList *list, char *alias) {
    // Initialize the semaphore if it is not initialized
    init_sem();
     
    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);
    
    // Delete user from the linked list
    int error_code = unregister_user(list, alias);
     
    // Writer releases the write semaphore
    sem_post(&writer_sem);
    
    return error_code;
}

/**
 * @brief Connect a user with the given alias.
 * @param list UserList
 * @param ip char*
 * @param port char*
 * @param alias char*
 * @return 0 -> Success, 1 -> User not found, 2 -> User already connected, 3 -> Error
 */
uint8_t list_connect_user(UserList *list, char *ip, char *port, char *alias) {
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);
    
    // Connect user in the linked list
    int error_code = connect_user(list, ip, port, alias);
    
    // Writer releases the write semaphore
    sem_post(&writer_sem);

    return error_code;
}


/**
 * @brief Disconnect a user with the given alias.
 * @param list UserList
 * @param ip char*
 * @param alias char*
 * @return 0 -> Success, 1 -> User not found, 2 -> User already disconnected, 3 -> Error
 */
uint8_t list_disconnect_user(UserList *list, char *ip, char *alias) {
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);
    
    // Disconnect user in the linked list
    int error_code = disconnect_user(list, ip, alias);
    
    // Writer releases the write semaphore
    sem_post(&writer_sem);
    
    return error_code;
}

/**
 * @brief Search for all connected users in the list.
 * @param list UserList
 * @param alias char*
 * @return ConnectedUsers struct with error_code: 0 -> Success, 1 -> User not connected, 2 -> User not found, 3 -> Error
 */
ConnectedUsers list_connected_users(UserList *list, char *alias) {
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Acquire the reader mutex
    pthread_mutex_lock(&reader_mut);

    // Increment the reader count
    reader_count++;

    // If it's the first reader, try to get the write semaphore
    if (reader_count == 1)
    {
        if (sem_trywait(&writer_sem) != 0)
        {
            pthread_mutex_unlock(&reader_mut);
            sem_post(&writer_sem);
            ConnectedUsers connected_users_result;
            // TODO: Cuidao con valgrind (no se si es necesario inicializarlo) -> connected_users_result.alias
            connected_users_result.size = 0;
            connected_users_result.error_code = 3;
            return connected_users_result;
        }
    }

    // Release the reader mutex
    pthread_mutex_unlock(&reader_mut);

    // Search for all connected users in the linked list
    ConnectedUsers connected_users_result = connected_users(list, alias);

    // Acquire the reader mutex
    pthread_mutex_lock(&reader_mut);

    // Decrement the reader count
    reader_count--;

    // If it's the last reader, release the write semaphore
    if (reader_count == 0)
    {
        sem_post(&writer_sem);
    }

    // Release the reader mutex
    pthread_mutex_unlock(&reader_mut);

    return connected_users_result;
}

/**
 * @brief Send a message from a user to another user.
 * @param list UserList
 * @param sourceAlias char*
 * @param destAlias char*
 * @param message char*
 * @return 0 -> Success, 1 -> Destination user not found, 2 -> Error
 */
uint8_t list_send_message(UserList *list, char *sourceAlias, char *destAlias, char *message) {
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);
    
    // Send message in the linked list
    int error_code = send_message(list, sourceAlias, destAlias, message);
    
    // Writer releases the write semaphore
    sem_post(&writer_sem);
    
    return error_code;
}

int list_display_user_list()
{
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);

    // Display the linked list
    display_users(user_list);

    // Writer releases the write semaphore
    sem_post(&writer_sem);

    return 0;
}

int list_display_pending_messages_list(char* alias)
{
    // Initialize the semaphore if it is not initialized
    init_sem();

    // Writer tries to get the write semaphore
    sem_wait(&writer_sem);

    // Display the linked list
    display_pending_messages(user_list, alias);

    // Writer releases the write semaphore
    sem_post(&writer_sem);

    return 0;
}

void request_delete_list()
{
    if (user_list == NULL) {
        return;
    }
    delete_user_list(user_list);
    free(user_list);
}

void print_connected_users(ConnectedUsers connected_users_result) {
    printf("\nConnected users (size: %d, error code: %d):\n", connected_users_result.size, connected_users_result.error_code);
    for (unsigned int i = 0; i < connected_users_result.size; i++) {
        printf("\t👤 Alias: %s\n", connected_users_result.alias[i]);
    }
    printf("\n");
}

// int main(void) {
//     list_init();

//     list_register_user(user_list, "127.0.0.1", "3000", "Carlos Iborra", "carlitos", "01/01/2000");
//     list_register_user(user_list, "127.0.0.1", "3000", "Rafael Contasti", "el veneco", "01/01/2002");
//     list_connect_user(user_list, "127.0.0.1", "3000", "el veneco");
    
//     list_unregister_user(user_list, "el veneco");
//     list_register_user(user_list, "127.0.0.1", "3000", "Rafael Contasti", "el veneco", "01/01/2002"); 
    
//     list_connect_user(user_list, "127.0.0.1", "3000", "carlitos");
//     list_disconnect_user(user_list, "127.0.0.1", "carlitos");
//     list_connect_user(user_list, "127.0.0.1", "3000", "carlitos");

//     list_register_user(user_list, "127.0.0.1", "3000", "Dibox", "diboxo1", "01/06/2002");
//     list_connect_user(user_list, "127.0.0.1", "3000", "diboxo1");

//     ConnectedUsers connected_users_result = list_connected_users(user_list, "carlitos");
//     print_connected_users(connected_users_result);

//     printf("Users after registering carlitos:\n");
//     list_display_user_list();

//     printf("Pending messages of el veneco:\n");
//     list_send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa cruck");
//     list_send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa maquina");
//     list_send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa fiera");
//     list_send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa mastodonte");
//     list_send_message(user_list, "carlitos", "el veneco", "Hola veneco, que pasa campeon");
//     list_display_pending_messages_list("el veneco");

//     request_delete_list();
// }
