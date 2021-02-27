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
#include "./message.hpp"

#define BUFSIZE 1000

struct client_data
{
   int tcp_sock;
   struct sockaddr_storage storage;
};

struct udp_connection
{
   int udp_sock;
   struct sockaddr_storage storage;
};

void usage()
{
   std::cout << "[!] Error executing server" << std::endl;
   std::cout << ">> Usage ./server <server_port>" << std::endl;
   std::cout << ">> Usage Example: ./server 56560" << std::endl;
   exit(EXIT_FAILURE);
};

void create_udp_socket(struct udp_connection *udp_data, int ss_family)
{
   if (ss_family == AF_INET)
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)(&udp_data->storage);
      addr4->sin_family = AF_INET;
      addr4->sin_addr.s_addr = INADDR_ANY;
      addr4->sin_port = htons(0);
   } 
   else if (ss_family== AF_INET6)
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)(&udp_data->storage);
      addr6->sin6_family = AF_INET6;
      addr6->sin6_addr = in6addr_any;
      addr6->sin6_port = htons(0);
   }

   udp_data->udp_sock = socket(udp_data->storage.ss_family,  SOCK_DGRAM, 0);
   if (udp_data->udp_sock == -1)
   {
      std::cout << "[!] Error creating UDP socket" << std::endl;
      exit(EXIT_FAILURE);
   }
}

int get_udp_port(struct udp_connection *udp_data)
{
   if (udp_data->storage.ss_family == AF_INET)
   {
      return ntohs(((struct sockaddr_in *)(&udp_data->storage))->sin_port);
   }
   else if (udp_data->storage.ss_family == AF_INET6)
   {
      return ntohs(((struct sockaddr_in6 *)(&udp_data->storage))->sin6_port);
   }

   return -1;
}

void recv_hello(struct client_data *c_data)
{
   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   size_t count = recv(c_data->tcp_sock, &r_buffer, BUFSIZE, 0);

   if (count > 0)
   {
      HELLO_OK_FIM_message_struct *msg = new HELLO_OK_FIM_message_struct;
      memcpy(msg, &r_buffer, sizeof(HELLO_OK_FIM_message_struct));

      if (msg->id != HELLO)
      {
         exit(EXIT_FAILURE);
      }
      else
      {
         std::cout << "Hello received" << std::endl;
      }

      delete msg;
   }
}

void send_connection(struct client_data *c_data, int udp_port)
{
   CONNECTION_message_struct * msg = new CONNECTION_message_struct;
   msg->id = CONNECTION;
   msg->port = udp_port;

   send(c_data->tcp_sock, msg, sizeof(CONNECTION_message_struct), 0);

   std::cout << "Connection Sent" << std::endl;
   delete msg;
}

void recv_infofile(struct client_data *c_data, char * file_name, uint64_t *file_size)
{
   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   size_t count = recv(c_data->tcp_sock, &r_buffer, BUFSIZE, 0);

   if (count > 0)
   {
      INFOFILE_message_struct *msg = new INFOFILE_message_struct;
      memcpy(msg, &r_buffer, sizeof(INFOFILE_message_struct));

      if (msg->id != INFO_FILE)
      {
         exit(EXIT_FAILURE);
      }
      else
      {
         std::cout << "Info File received" << std::endl;
      }

      memcpy(file_name, &(msg->file_name), 15 * sizeof(char));
      *file_size = msg->file_size;

      delete msg;
   }
}

void send_ok(struct client_data *c_data)
{
   HELLO_OK_FIM_message_struct * msg = new HELLO_OK_FIM_message_struct;
   msg->id = OK;

   send(c_data->tcp_sock, msg, sizeof(HELLO_OK_FIM_message_struct), 0);

   std::cout << "Ok Sent" << std::endl;
   delete msg;
}

void send_fim(struct client_data *c_data)
{
   HELLO_OK_FIM_message_struct * msg = new HELLO_OK_FIM_message_struct;
   msg->id = FIM;

   send(c_data->tcp_sock, msg, sizeof(HELLO_OK_FIM_message_struct), 0);

   std::cout << "Fim Sent" << std::endl;
   delete msg;
}

void * client_handling_thread(void *data)
{
   struct client_data *c_data = (struct client_data *)data;
   // struct sockaddr *c_addr = (struct sockaddr *)(&c_data->storage);

   recv_hello(c_data);

   struct udp_connection *udp_data = new udp_connection;
   create_udp_socket(udp_data, c_data->storage.ss_family);

   if (0 != bind(udp_data->udp_sock, (struct sockaddr *)(&udp_data->storage), sizeof(udp_data->storage)))
   {
      std::cout << "[!] Error bind" << std::endl;
      exit(EXIT_FAILURE);
   }

   socklen_t udpaddrlen = sizeof(udp_data->storage);
   getsockname(udp_data->udp_sock, (struct sockaddr *)(&udp_data->storage), &udpaddrlen);

   int udp_port = get_udp_port(udp_data);
   if (udp_port == -1)
   {
      std::cout << "[!] Error getting UDP socket port" << std::endl;
      exit(EXIT_FAILURE);
   }

   send_connection(c_data, udp_port);

   char file_name[15];
   uint64_t file_size;
   recv_infofile(c_data, file_name, &file_size);

   send_ok(c_data);

   send_fim(c_data);

   close(c_data->tcp_sock);
   close(udp_data->udp_sock);
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
      addr6->sin6_addr = in6addr_any;
      addr6->sin6_port = htons(port);

      std::cout << ">> 128-bit IP address (IPv6)" << std::endl;
   } 
   else
   {
      usage();
   }

   int s_socket = socket(storage.ss_family,  SOCK_STREAM, 0);
   if (s_socket == -1)
   {
      std::cout << "[!] Error while creating TCP socket" << std::endl;
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
      int tcp_sock = accept(s_socket, (struct sockaddr *)(&client_storage), &caddrlen);
      std::cout << ">> Client connected from: " << getAddrStr((struct sockaddr *)(&client_storage)) << " " << getAddrPort((struct sockaddr *)(&client_storage)) << std::endl;

      if (tcp_sock == -1)
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

      client_data_thread->tcp_sock = tcp_sock;
      memcpy(&(client_data_thread->storage), &client_storage, sizeof(client_storage));

      pthread_t tid;
      pthread_create(&tid, NULL, client_handling_thread, client_data_thread);

      pthread_join(tid, NULL);
   }
   
   exit(EXIT_SUCCESS);
}
