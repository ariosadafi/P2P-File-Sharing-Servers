Distributed Programming Course
Politecnico di Torino
==============================
June 2013

Develop a server for the simple file transfer protocol described in exercise 2.3 according to the following specifications:

1. The server receives exactly two arguments on the command line: the TCP port number to which it listens and
the name of a configuration file.

2. The configuration file is a text file that lists zero or more servers, one per line. Each line is composed of two
ASCII strings separated by a space. The first string can be either the IPv4 address of the server host in dotted
decimal notation or the name of the server host. The second string is the server TCP port number.

3. The server interacts with the clients exactly as specified in exercise 2.3.
4. 
4. When the server receives a request for a file, it looks for the file in its local current directory. If the file is not
found, the server tries to get the file from one of the servers listed in its configuration file (the server interacts
with the other servers using the same protocol, but playing the client role). The server tries to contact the
servers in the configuration file one by one, in the same order as they appear in the file, until one of them
returns the file or the list scanning terminates.

5. If one of the servers listed in the configuration file returns the requested file, the server forwards the received
file to the client that requested it. File forwarding must be done with minimum latency, i.e. the server must
forward file data while they arrive from the other server (without waiting for the reception of the whole file).

6. Only if no server listed in the configuration file returns the requested file (either because the server is nonreachable
or because it returns an ERR response) the server responds to the client with the ERR response.

7. If, while receiving the file from another server, that server closes the connection prematurely, the server to be
implemented must do the same with the client, i.e. close the connection with the client prematurely.

8. The server must be developed for the Linux OS.

9. The server must be able to serve at least 5 clients simultaneously.
