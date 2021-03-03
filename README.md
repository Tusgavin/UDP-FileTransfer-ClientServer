# UDP-FileTransfer-ClientServer
A simple client and server for upload documents using UDP protocol

## Commands

To compile the program, go to /src and run the command:

```properties 
make 
```

The commands creates a folder client with the executable 'cliente' and a folder server with the executable 'servidor'.

In the 'server' folder start the server:

```properties
./servidor <port> 
```

Add a file 'teste.txt', for example, in the 'client' folder and run:

```properties
./cliente <IPv4-address> <server-port> <file_name>
```

## Result

The file will be uploaded to server and will be stored in the same folder as the executable 'servidor' is located.
