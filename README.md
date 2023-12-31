# 📢 Message Delivery with Sockets using Python as the Interface and C as the Backend

Welcome to our GitHub repository for a unique message delivery system that combines the flexibility of Python and the efficiency of C. This project demonstrates a client-server architecture where Python serves as the client interface and C as the backend.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

What things you need to install the software and how to install them:

- Python 3.x
- GCC Compiler for C
- Make

### Installing

A step by step series of examples that tell you how to get a development env running:

1. Clone the repository to your local machine.
2. Ensure Python 3.x and GCC Compiler are installed on your system.
3. Navigate to the project directory.

## Running the Applications

To run the applications, use the following commands:

### Run Python Client:

```bash
python3 ./client.py -s localhost -p 8888
```

### Run Server Application:

```bash
make proxy && ./servidor -p 8888
```

### Run Web Service Server:

```bash
python3 ws-text.py
```

## Project Structure

### Client-Server Diagram

The project includes a diagram illustrating the interaction and dependencies between the C files and the Python client, along with the header files:

![Client-Server Diagram](https://github.com/carlosiborra/message-delivery-python-c/blob/main/sockets-c.png)

### Web Services

A web service is implemented to format messages before they are sent. This ensures correct spacing and formatting in all communications.

## Design and Implementation

### Data Structure

- **Client List**: Implemented as a linked list storing client data including IP, port, and message history.
- **Message List**: A linked list for each client storing pending messages.

### Code Style

The code is well-commented for easy understanding and maintenance, both in C and Python parts.

### Communication Protocol

TCP sockets are used to ensure reliable message and data transfer.

## Compilation and Execution

### Compilation

To compile the server and its C dependencies, use:

```bash
make proxy
```

### Execution

Refer to the "Running the Applications" section above.

### Deletion

To delete the server executable, run:

```bash
make clean
```

## Testing

The project includes extensive testing for various functionalities, ensuring robustness and reliability.

## Conclusions

This project provides a comprehensive experience in developing a messaging application using TCP sockets, combining Python and C.

## Valgrind Analysis

Memory management is thoroughly analyzed using Valgrind, ensuring no memory leaks in the server's operation.
