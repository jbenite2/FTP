# Semester Project (Skeleton Code)
Name: Jose Benitez
NDID: jbenite2

### High Level Design
#### Server 
- Binds to a specific port and IP address using the bind() system call
- Listen for incoming connections using the listen() system call
- Inside a while loop for durability, accept the connections that come in. 
- Populate a buffer of set size until no more data is passed through the connection.
- Write from the buffer to an output file.
- Close the connection
#### Client
- After the arguments are parsed and error checking is done, a socket is created for the first time
- Like the server, the client has to bind to a port
- It tries to connect to the server using the created socket
- Once the connection is established, it'll try to send the data in chunks of 1024 until there is nothing left. 
- The connection is closed at the end.

### Problems
- The biggest problems I ran into had to do with not being able to replicate test #15s condition to test manually. I asked for help from the TAs to make sure I was following the correct procedure. 

### List of any additional libraries used
- <sys/types.h>
- <sys/socket.h>
- <netinet/in.h>
- <arpa/inet.h>
- <netdb.h>
- <sys/time.h>

### Acknowledgement of any online tutorials or code example (except class website) you have been using.
I mostly used the man pages for guidance and stack overflow when running into errors. 
