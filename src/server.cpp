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
#include <algorithm>

#include "./utils.hpp"

#define BUFSIZE 500

struct client_data
{
   int c_sock;
   struct sockaddr_storage storage;
};

void usage()
{
   std::cout << "[!] Error executing server" << std::endl;
   std::cout << ">> Usage ./server <server_port>" << std::endl;
   std::cout << ">> Usage Example: ./server 56560" << std::endl;
   exit(EXIT_FAILURE);
};

void * client_handling_thread(void *data)
{
   struct client_data *c_data = (struct client_data *)data;
   // struct sockaddr *c_addr = (struct sockaddr *)(&c_data->storage);

   while(1)
   {
      char r_buffer[BUFSIZE];
      memset(r_buffer, 0, BUFSIZE);
      size_t count = recv(c_data->c_sock, r_buffer, BUFSIZE, 0);

      if (count > 0)
      {
         std::cout << "Received: " << std::string(r_buffer) << std::endl;
      }

      std::string pure_message = std::string(r_buffer).substr(0, std::string(r_buffer).length() - 1);

      if (pure_message == "EXIT_SERVER")
      {
         exit(EXIT_SUCCESS);
      }
      else if (pure_message == "Give me a life signal")
      {
         std::string Life_Signal = "I am here";
         send(c_data->c_sock, Life_Signal.c_str(), Life_Signal.length(), 0);
      }
   }

   pthread_exit(EXIT_SUCCESS);
};

int main(int argc, char *argv[])
{
   if (argc < 2)
   {
      usage();
   }

   /* Get port and type of IP */
   int port = atoi(argv[1]);
   std::string type_of_IP = "v4";

   /* Create server socket */
   struct sockaddr_storage storage;
   memset(&storage, 0, sizeof(storage));

   if (type_of_IP == "v4")
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)(&storage);
      addr4->sin_family = AF_INET;
      addr4->sin_addr.s_addr = INADDR_ANY;
      addr4->sin_port = htons(port);

      std::cout << ">> Server 32-bit IP address (IPv4)" << std::endl;
   } 
   else if (type_of_IP == "v6")
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)(&storage);
      addr6->sin6_family = AF_INET6;
      addr6->sin6_port = htons(port);
      addr6->sin6_addr = in6addr_any;

      std::cout << ">> 128-bit IP address (IPv6)" << std::endl;
   } 
   else
   {
      usage();
   }

   int s_socket = socket(storage.ss_family,  SOCK_STREAM, 0);
   if (s_socket == -1)
   {
      std::cout << "[!] Error while creating socket" << std::endl;
      exit(EXIT_FAILURE);
   }

   int enable = 1;
   if (0 != setsockopt(s_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)))
   {
      std::cout << "[!] Error reusing port " << port << std::endl;
      exit(EXIT_FAILURE);
   }

   /* Bind */
   if (0 != bind(s_socket, (struct sockaddr *)(&storage), sizeof(storage)))
   {
      std::cout << "[!] Error bind" << std::endl;
      exit(EXIT_FAILURE);
   }

   /* Listen */
   if (0 != listen(s_socket, 10))
   {
      std::cout << "[!] Error listen" << std::endl;
      exit(EXIT_FAILURE);
   }

   std::cout << ">> Server waiting for connections..." << std::endl;

   /* Accept */
   while(1)
   {
      struct sockaddr_storage client_storage;
      socklen_t caddrlen = sizeof(client_storage);
      int c_sock = accept(s_socket, (struct sockaddr *)(&client_storage), &caddrlen);
      std::cout << ">> Client connected from: " << getAddrStr((struct sockaddr *)(&client_storage)) << " " << getAddrPort((struct sockaddr *)(&client_storage)) << std::endl;

      if (c_sock == -1)
      {
         std::cout << "[!] Error accept" << std::endl;
         exit(EXIT_FAILURE);
      }

      struct client_data *client_data_thread = (struct client_data *)malloc(sizeof(*client_data_thread));
      if (!client_data_thread)
      {
         std::cout << "[!] Error allocating memory" << std::endl;
         exit(EXIT_FAILURE);
      }

      client_data_thread->c_sock = c_sock;
      memcpy(&(client_data_thread->storage), &client_storage, sizeof(client_storage));

      pthread_t tid;
      pthread_create(&tid, NULL, client_handling_thread, client_data_thread);
   }
   
   exit(EXIT_SUCCESS);
}
