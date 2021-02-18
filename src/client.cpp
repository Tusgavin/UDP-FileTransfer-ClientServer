#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>

#define BUFSIZE 500

struct server_data {
   int sock;
   struct sockaddr_storage storage;
};

void usage()
{
   std::cout << "[!] Error executing client" << std::endl;
   std::cout << "Usage ./client <server_ip> <port> <file_name>" << std::endl;
   std::cout << "Usage Example: ./client 127.0.0.1 56560 file.doc" << std::endl;
   exit(EXIT_FAILURE);
}

void * client_receiving_thread(void *data)
{
   struct server_data *s_data = (struct server_data *)data;
   //struct sockaddr *s_addr = (struct sockaddr *)(&s_data->storage);

   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   while(1)
   {
      size_t count_recv = recv(s_data->sock, r_buffer, BUFSIZE, 0);
      if (count_recv > 0)
      {
         std::cout << "Received: " << std::string(r_buffer) << std::endl;
      }
      else if (count_recv == 0)
      {
         exit(EXIT_SUCCESS);
      }

      memset(r_buffer, 0, BUFSIZE);
   }

   pthread_exit(EXIT_SUCCESS);
};

void * client_sending_thread(void *data)
{
   struct server_data *s_data = (struct server_data *)data;

   std::string buffer;
   
   while(std::getline(std::cin, buffer))
   {
      if(std::string(buffer).length() > 0)
      {
         buffer = buffer + '\n';
         size_t count_send = send(s_data->sock, buffer.c_str(), std::string(buffer).length(), 0);
         if (count_send != std::string(buffer).length())
         {
            std::cout << "[!] Error while sending message" << std::endl;
         }
         if (buffer == "!EXIT")
         {
            break;
         }
      }
   }

   pthread_exit(EXIT_SUCCESS);
};

int main(int argc, char *argv[])
{
   if (argc < 3)
   {
      usage();
   }

   /* Get port and server ip */
   int port = atoi(argv[2]);
   std::string server_ip = std::string(argv[1]);

   /* Try connecting to the server */
   std::cout << ">> Connecting to server: " << server_ip << " / port: " << port << " ..." << std::endl;

   struct sockaddr_storage storage;
   memset(&storage, 0, sizeof(storage));

   /*  Check if IPv4  */
   struct in_addr inaddr4;
   if (inet_pton(AF_INET, server_ip.c_str(), &inaddr4))
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)(&storage);
      addr4->sin_family = AF_INET;
      addr4->sin_addr = inaddr4;
      addr4->sin_port = htons(port);

      std::cout << ">> 32-bit IP address (IPv4)" << std::endl;
   }

   /*  Check if IPv6  */
   struct in6_addr inaddr6;
   if (inet_pton(AF_INET6, server_ip.c_str(), &inaddr6))
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)(&storage);
      addr6->sin6_family = AF_INET6;
      addr6->sin6_port = htons(port);
      memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));

      std::cout << ">> 128-bit IP address (IPv6)" << std::endl;
   }

   int sock = socket(storage.ss_family,  SOCK_STREAM, 0);
   if (sock == -1)
   {
      std::cout << "[!] Error while creating socket" << std::endl;
   }

   if (0 != connect(sock, (struct sockaddr *)(&storage), sizeof(storage)))
   {
      std::cout << "[!] Error while connecting to server" << std::endl;
      exit(EXIT_FAILURE);
   }

   std::cout << ">> Connection established with success" << std::endl;

   struct server_data *server_data_thread = (struct server_data *)malloc(sizeof(*server_data_thread));
   if (!server_data_thread)
   {
      std::cout << "[!] Error allocating memory" << std::endl;
      exit(EXIT_FAILURE);
   }

   server_data_thread->sock = sock;

   memcpy(&(server_data_thread->storage), &storage, sizeof(storage));

   pthread_t send_thread;
   pthread_create(&send_thread, NULL, client_sending_thread, server_data_thread);

   pthread_t recv_thread;
   pthread_create(&recv_thread, NULL, client_receiving_thread, server_data_thread);

   pthread_join(recv_thread, NULL);

   close(sock);
   exit(EXIT_SUCCESS);
};
