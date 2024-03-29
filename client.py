import subprocess
import sys
import PySimpleGUI as sg
from enum import Enum
import argparse
import socket
import threading
import zeep

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None
    _listening_sock = None
    _listening_port = -1
    
    # Web service client attribute
    _web_host = "localhost:8000"

    # ******************** METHODS *******************

    # *
    # * @return the socket to the server
    @staticmethod
    def create_socket_and_connect():
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((client._server, client._port))
        return sock
    
    @staticmethod
    def readNumber(sock):
        a = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break;
            a += msg.decode()

        return(int(a,10))
    
    @staticmethod
    def readString(sock):
        a = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break;
            a += msg.decode()

        return(a)


    # *
    # * @param user - User name to register in the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    @staticmethod
    def  register(user, window):
        try: 
            sock = client.create_socket_and_connect()

            # Indicate the server that we want to register
            sock.sendall("REGISTER".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: username, alias and date
            sock.sendall(client._username.encode())
            sock.sendall(b'\0')
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')
            sock.sendall(client._date.encode())
            sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)

            # Close the socket
            sock.close()
            
            if (response == b'\x00'):
                window['_SERVER_'].print("s> REGISTER OK")
                return client.RC.OK
            elif (response == b'\x01'):
                window['_SERVER_'].print("s> USERNAME IN USE")
                return client.RC.USER_ERROR
            else:
                window['_SERVER_'].print("s> REGISTER FAIL")
                return client.RC.ERROR
        except Exception as _:
            window['_SERVER_'].print("s> REGISTER FAIL")
            return client.RC.ERROR

    # *
    # * @param user - User name to unregister from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def  unregister(user, window):
        try: 
            sock = client.create_socket_and_connect()

            # Indicate the server that we want to register
            sock.sendall("UNREGISTER".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: alias
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)

            # Close the socket
            sock.close()
            
            if (response == b'\x00'):
                window['_SERVER_'].print("s> UNREGISTER OK")
                return client.RC.OK
            elif (response == b'\x01'):
                window['_SERVER_'].print("s> USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            else:
                window['_SERVER_'].print("s> UNREGISTER FAIL")
                return client.RC.ERROR
        except Exception as _:
            window['_SERVER_'].print("s> UNREGISTER FAIL")
            return client.RC.ERROR

    @staticmethod
    def listen(sock, window):
        # Start listening for messages
        sock.listen(1)

        while True:
            try:
                # Accept a connection
                C_socket, C_address = sock.accept()
                print(f"Connection from {C_address} has been established!")

                cadena = client.readString(C_socket)
                print(f"Cadena: {cadena}")
                if cadena == "SEND_MESSAGE":
                    sourceAlias = client.readString(C_socket)
                    messageId = client.readString(C_socket)
                    message = client.readString(C_socket)
                    window['_SERVER_'].print(f"s> MESSAGE {messageId} FROM {sourceAlias} \n{message}\nEND")
                elif cadena == "SEND_MESS_ACK":
                    messageId = client.readString(C_socket)
                    print(f"message id: {messageId}")
                    window['_SERVER_'].print(f"s> SEND MESSAGE {messageId} OK")

                # Close the connection with the client
                C_socket.close()

            except Exception as _:
                sock.close()
                return

        # Close the listening socket
        sock.close()

    # *
    # * @param user - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred
    @staticmethod
    def  connect(user, window):
        try: 
            sock = client.create_socket_and_connect()

            # Check if we have already created a socket to listen for messages
            # If not, create it
            if client._listening_port == -1:
                # Create a socket to listen for messages
                client._listening_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client._listening_sock.bind(('', 0))
                client._listening_port = client._listening_sock.getsockname()[1]

            # Indicate the server that we want to register
            sock.sendall("CONNECT".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: alias, port where we are listening for the messages
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')

            sock.sendall(str(client._listening_port).encode())
            sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)

            if (response == b'\x00'):
                print(f"Listening port: {client._listening_port}")

                listen_thread = threading.Thread(target=client.listen, args=(client._listening_sock, window))
                listen_thread.daemon = True # Stop the thread when the main thread ends
                listen_thread.start()

                sock.close()

                window['_SERVER_'].print("s> CONNECT OK")

                return client.RC.OK
            elif (response == b'\x01'):
                sock.close()
                window['_SERVER_'].print("s> CONNECT FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif (response == b'\x02'):
                sock.close()
                window['_SERVER_'].print("s> USER ALREADY CONNECTED")
                return client.RC.USER_ERROR
            else:
                sock.close()
                window['_SERVER_'].print("s> CONNECT FAIL")
                return client.RC.ERROR
        except Exception as _:
            window['_SERVER_'].print("s> CONNECT FAIL")
            return client.RC.ERROR


    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def  disconnect(user, window):
        try:
            sock = client.create_socket_and_connect()

            # Indicate the server that we want to register
            sock.sendall("DISCONNECT".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: alias
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)

            # Close the socket
            sock.close()
            
            if (response == b'\x00'):
                window['_SERVER_'].print("s> DISCONNECT OK")

                # Stop listening for messages
                client._listening_sock.close()
                client._listening_port = -1
                return client.RC.OK
            elif (response == b'\x01'):
                window['_SERVER_'].print("s> DISCONNECT FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            elif (response == b'\x02'):
                window['_SERVER_'].print("s> DISCONNECT FAIL / USER NOT CONNECTED")
                return client.RC.USER_ERROR
            else:
                window['_SERVER_'].print("s> DISCONNECT FAIL")
                return client.RC.ERROR

        except Exception as _:
            window['_SERVER_'].print("s> DISCONNECT FAIL")
            return client.RC.ERROR
            
        

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  send(user, message, window):
        try:
            sock = client.create_socket_and_connect()

            # Indicate the server that we want to send a message
            sock.sendall("SEND".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: sender alias
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')
            
            # Sending the rest of the data: receiver alias
            sock.sendall(user.encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: message   
            # Call to the text web service
            wsdl_url = f"http://{client._web_host}/?wsdl"
            soap = zeep.Client(wsdl=wsdl_url)
            new_message = soap.service.convert_text(message)
            new_message = new_message[:255]
            sock.sendall(new_message.encode())
            sock.sendall(b'\0')

            # sock.sendall(message.encode())
            # sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)
            
            if (response == b'\x00'):
                # Read the message id
                messageId = client.readString(sock)
                sock.close()

                window['_SERVER_'].print(f"s> SEND OK - MESSAGE {messageId}")

                return client.RC.OK
            elif (response == b'\x01'):
                window['_SERVER_'].print("s> SEND FAIL / USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            else:
                window['_SERVER_'].print("s> SEND FAIL")
                return client.RC.ERROR

        except Exception as _:
            window['_SERVER_'].print("s> SEND FAIL")
            return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent

    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  sendAttach(user, message, file, window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  connectedUsers(window):
        try: 
            sock = client.create_socket_and_connect()

            # Indicate the server that we want to register
            sock.sendall("CONNECTEDUSERS".encode())
            sock.sendall(b'\0')

            # Sending the rest of the data: alias
            sock.sendall(client._alias.encode())
            sock.sendall(b'\0')

            # Receive the response from the server
            response = sock.recv(1)
            
            if (response == b'\x00'):
                # Read the number of connected users and then read as many aliases as connected users are (joining them by commas)
                numConnUsers = client.readNumber(sock)
                aliases = ', '.join([client.readString(sock) for _ in range(numConnUsers)])
                sock.close()    
                
                window['_SERVER_'].print(f"s> CONNECTED USERS ({numConnUsers} users connected) OK - {aliases}")
                return client.RC.OK
            elif (response == b'\x01'):
                sock.close()
                window['_SERVER_'].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
                return client.RC.USER_ERROR
            else:
                sock.close()
                window['_SERVER_'].print("s> CONNECTED USERS FAIL")
                return client.RC.ERROR
            
    
        except Exception as _:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")
            return client.RC.ERROR


    @staticmethod
    def window_register():
        layout_register = [[sg.Text('Ful Name:'),sg.Input('Text',key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Alias:'),sg.Input('Text',key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Date of birth:'),sg.Input('',key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                            sg.CalendarButton("Select Date",close_when_date_chosen=True, target="_REGISTERDATE_", format='%d/%m/%Y',size=(10,1))],
                            [sg.Button('SUBMIT', button_color=('white', 'blue'))]
                            ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)
        choice = None

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if(values['_REGISTERNAME_'] == 'Text' or values['_REGISTERNAME_'] == '' or values['_REGISTERALIAS_'] == 'Text' or values['_REGISTERALIAS_'] == '' or values['_REGISTERDATE_'] == ''):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()


    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;

        client._server = args.s
        client._port = args.p

        return True


    def main(argv):

        if (not client.parseArguments(argv)):
            client.usage()
            exit()

        lay_col = [[sg.Button('REGISTER',expand_x=True, expand_y=True),
                sg.Button('UNREGISTER',expand_x=True, expand_y=True),
                sg.Button('CONNECT',expand_x=True, expand_y=True),
                sg.Button('DISCONNECT',expand_x=True, expand_y=True),
                sg.Button('CONNECTED USERS',expand_x=True, expand_y=True)],
                [sg.Text('Dest:'),sg.Input('User',key='_INDEST_', do_not_clear=True, expand_x=True),
                sg.Text('Message:'),sg.Input('Text',key='_IN_', do_not_clear=True, expand_x=True),
                sg.Button('SEND',expand_x=True, expand_y=False)],
                [sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                sg.Button('SENDATTACH',expand_x=True, expand_y=False)],
                [sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True),
                sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True)],
                [sg.Button('QUIT', button_color=('white', 'red'))]
            ]


        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000,400))
        window.bind("<Escape>", "-ESCAPE-")


        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            #if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
             #   window['_CLIENT_'].print("c> No text inserted")
             #   continue

            if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None) and (event != 'REGISTER'):
                sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                continue

            if (event == 'REGISTER'):
                client.window_register()

                if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._alias, window)

            elif (event == 'UNREGISTER'):
                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                client.unregister(client._alias, window)


            elif (event == 'CONNECT'):
                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)


            elif (event == 'DISCONNECT'):
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    client.send(values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)


            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")
