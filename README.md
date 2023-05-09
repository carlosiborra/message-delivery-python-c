# Ejercicio2-SocketsTCP

## Comandos de compilación

- Compilar el servidor: `make servidor.out && ./servidor.out`
- Compilar el cliente: `make cliente.out && ./cliente.out`

  El -lrt se usa para que el programa pueda usar las funciones de tiempo real.
- Para mirar la cola de mensajes: `ls /dev/mqueue/`
- Para leer el contenido de la cola de mensajes: `cat /dev/mqueue/<nombre de la cola>`

- Para eliminar la cola de mensajes: `rm /dev/mqueue/<nombre de la cola>`
  > `rm /dev/mqueue/mq_server`

  > `rm /dev/mqueue/mq_client_0`

Ejecutar app. python:
> `python ./client.py -s localhost -p 3000`
