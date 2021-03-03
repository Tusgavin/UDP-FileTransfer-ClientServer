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
#include <fstream>
#include <filesystem>
#include <cstdio>

#include "./message.hpp"

#define BUFSIZE 1000

char file_name[15];

struct server_data
{
   int sock;
   struct sockaddr_storage storage;
};

struct udp_connection
{
   int udp_sock;
   struct sockaddr_storage storage;
};

void usage()
{
   std::cout << "[!] Error executing client" << std::endl;
   std::cout << "Usage ./client <server_ip> <port> <file_name>" << std::endl;
   std::cout << "Usage Example: ./client 127.0.0.1 56560 file.doc" << std::endl;
   exit(EXIT_FAILURE);
}

uint64_t get_file_size()
{
   std::ifstream file(file_name, std::ifstream::in | std::ifstream::binary);

   if (!file.is_open())
   {
      return -1;
   }

   file.seekg(0, std::ios::end);
   uint64_t file_size = file.tellg();
   file.close();

   return file_size;
}

void create_udp_socket(struct udp_connection *udp_data, int port, int ss_family)
{
   if (ss_family == AF_INET)
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)(&udp_data->storage);
      addr4->sin_family = AF_INET;
      addr4->sin_addr.s_addr = INADDR_ANY;
      addr4->sin_port = htons(port);
   }
   else if (ss_family == AF_INET6)
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)(&udp_data->storage);
      addr6->sin6_family = AF_INET6;
      addr6->sin6_addr = in6addr_any;
      addr6->sin6_port = htons(port);
   }

   udp_data->udp_sock = socket(udp_data->storage.ss_family, SOCK_DGRAM, 0);
   if (udp_data->udp_sock == -1)
   {
      std::cout << "[!] Error creating UDP socket" << std::endl;
      exit(EXIT_FAILURE);
   }
}

void send_hello(struct server_data *s_data)
{
   HELLO_OK_FIM_message_struct *msg = new HELLO_OK_FIM_message_struct;
   msg->id = HELLO;

   send(s_data->sock, msg, sizeof(CONNECTION_message_struct), 0);

   std::cout << "Hello Sent" << std::endl;

   delete msg;
}

void recv_connection(struct server_data *s_data, struct udp_connection *udp_data)
{
   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   size_t count = recv(s_data->sock, &r_buffer, BUFSIZE, 0);

   if (count > 0)
   {
      CONNECTION_message_struct *conn_msg = new CONNECTION_message_struct;
      memcpy(conn_msg, &r_buffer, sizeof(CONNECTION_message_struct));

      if (conn_msg->id != CONNECTION)
      {
         exit(EXIT_FAILURE);
      }
      else
      {
         std::cout << "Connection received" << std::endl;
      }

      int ss_family = s_data->storage.ss_family;

      create_udp_socket(udp_data, conn_msg->port, ss_family);

      delete conn_msg;
   }
}

void send_infofile(struct server_data *s_data)
{
   INFOFILE_message_struct *msg = new INFOFILE_message_struct;
   msg->id = INFO_FILE;

   uint64_t file_size = get_file_size();

   msg->file_size = file_size;
   memcpy(&(msg->file_name), &file_name, sizeof(file_name));

   send(s_data->sock, msg, sizeof(INFOFILE_message_struct), 0);

   std::cout << "Info File sent" << std::endl;

   delete msg;
}

void recv_ok(struct server_data *s_data)
{
   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   size_t count = recv(s_data->sock, &r_buffer, BUFSIZE, 0);

   if (count > 0)
   {
      HELLO_OK_FIM_message_struct *msg = new HELLO_OK_FIM_message_struct;
      memcpy(msg, &r_buffer, sizeof(HELLO_OK_FIM_message_struct));

      if (msg->id != OK)
      {
         exit(EXIT_FAILURE);
      }
      else
      {
         std::cout << "Ok received" << std::endl;
      }

      delete msg;
   }
}

void recv_fim(struct server_data *s_data)
{
   char r_buffer[BUFSIZE];
   memset(r_buffer, 0, BUFSIZE);

   size_t count = recv(s_data->sock, &r_buffer, BUFSIZE, 0);

   if (count > 0)
   {
      HELLO_OK_FIM_message_struct *msg = new HELLO_OK_FIM_message_struct;
      memcpy(msg, &r_buffer, sizeof(HELLO_OK_FIM_message_struct));

      if (msg->id != FIM)
      {
         exit(EXIT_FAILURE);
      }
      else
      {
         std::cout << "Fim received" << std::endl;
      }

      delete msg;
   }
}

void send_file_to_server(struct udp_connection *udp_data)
{
   std::ifstream file(file_name, std::ios::binary | std::ios::ate);

   uint64_t file_size = get_file_size();
   char buffer[file_size];

   file.seekg(0, std::ios::beg);

   if(file.read(buffer, file_size))
   {
      int s = sendto(udp_data->udp_sock, buffer, file_size, 0, (struct sockaddr *)(&udp_data->storage), sizeof(udp_data->storage));

      if (s == -1)
      {
         std::cout << "[!] Cannot send file" << std::endl;
         exit(EXIT_FAILURE);
      }

      bzero(buffer, file_size);

      strcpy(buffer, "END");
      sendto(udp_data->udp_sock, buffer, file_size, 0, (struct sockaddr *)(&udp_data->storage), sizeof(udp_data->storage));
   }

   file.close();
}

void *udp_file_thread(void *data)
{
   struct udp_connection *upd_data = (struct udp_connection *)data;
   send_file_to_server(upd_data);

   close(upd_data->udp_sock);
   pthread_exit(EXIT_SUCCESS);
}

void *client_thread(void *data)
{
   struct server_data *s_data = (struct server_data *)data;

   send_hello(s_data);

   struct udp_connection *udp_thread_data = new udp_connection;
   recv_connection(s_data, udp_thread_data);

   send_infofile(s_data);

   recv_ok(s_data);

   pthread_t udp_thread;
   pthread_create(&udp_thread, NULL, udp_file_thread, udp_thread_data);

   recv_fim(s_data);

   close(s_data->sock);
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
   memcpy(&file_name, argv[3], 15 * sizeof(char));

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

   int sock = socket(storage.ss_family, SOCK_STREAM, 0);
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

   pthread_t tid;
   pthread_create(&tid, NULL, client_thread, server_data_thread);

   pthread_join(tid, NULL);

   close(sock);
   exit(EXIT_SUCCESS);
};