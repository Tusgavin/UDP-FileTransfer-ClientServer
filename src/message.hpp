#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <stdint.h>

#define UDP_PORT_SIZE 4
#define FILE_NAME_SIZE 15

#define HELLO 1
#define CONNECTION 2
#define INFO_FILE 3
#define OK 4
#define FIM 5
#define FILE 6
#define ACK 7

#pragma pack(1)
typedef struct
{
   uint16_t id;
} HELLO_OK_FIM_message_struct;
#pragma pack()

#pragma pack(1)
typedef struct
{
   uint16_t id;
   uint32_t port;
} CONNECTION_message_struct;
#pragma pack()

#pragma pack(1)
typedef struct
{
   uint16_t id;
   char file_name[15];
   uint64_t file_size;
} INFOFILE_message_struct;
#pragma pack()

#pragma pack(1)
typedef struct
{
   uint16_t id;
   char sequence_number[4];
   uint16_t payload_size;
   char payload_file[1000];
} FILE_message_struct;
#pragma pack()

#pragma pack(1)
typedef struct
{
   uint16_t id;
   char sequence_number[4];
} ACK_message_struct;
#pragma pack()

#endif //MESSAGE_HPP