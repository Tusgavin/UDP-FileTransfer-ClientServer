#include "./utils.hpp"

int getAddrPort(const struct sockaddr *addr)
{
   int port; 

   if (addr->sa_family == AF_INET)
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
      port = ntohs(addr4->sin_port);
      return port;
   }
   else if (addr->sa_family == AF_INET6)
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
      port = ntohs(addr6->sin6_port);
      return port;
   }
   else 
   {
      std::cout << "Protocolo indefinido" << std::endl;
      return -1;
   }
}

std::string getAddrStr(const struct sockaddr *addr)
{
   char addrstr[INET6_ADDRSTRLEN + 1] = "";

   if (addr->sa_family == AF_INET)
   {
      struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
      if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1)) 
      {
         std::cout << "[!] Erro getting Addr String" << std::endl;
      }
      
      return std::string(addrstr);
   }
   else if (addr->sa_family == AF_INET6)
   {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
      if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN + 1)) 
      {
         std::cout << "[!] Erro getting Addr String" << std::endl;
      }
      
      return std::string(addrstr);
   }
   else 
   {
      return "Protocolo indefinido";
   }
}