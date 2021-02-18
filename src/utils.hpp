#ifndef UTILS_HPP
#define UTILS_HPP

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

int getAddrPort(const struct sockaddr *addr);

std::string getAddrStr(const struct sockaddr *addr);

#endif  // UTILS_HPP