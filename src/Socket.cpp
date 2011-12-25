#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <string>

#include "Error.h"

#include "Socket.h"

namespace Omegle
{
  Socket::~Socket()
  {
    if(IsConnected())
    {
      Disconnect();
    }
  }

  void Socket::Connect(const std::string& address, const std::string& port)
  { 
    if(IsConnected())
    {
      Disconnect();
    }

    struct addrinfo hints;
    struct addrinfo* res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((getaddrinfo(address.c_str(), port.c_str(), &hints, &res) != 0) ||
      ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) ||
      (connect(sock, res->ai_addr, res->ai_addrlen) != 0))
    {
      freeaddrinfo(res);
      throw SocketError(strerror(errno));
    }

    freeaddrinfo(res);
  }

  void Socket::Disconnect()
  {
    if(IsConnected())
    {
      close(sock);
    }
  }

  bool Socket::IsConnected()
  {
    if(fcntl(sock, F_GETFL) == -1)
    {
      assert(errno == EBADF);
      return false;
    }
    return true;
  }

  size_t Socket::Recv(void* const buf, const size_t len, const bool blocking) 
  {
    if(blocking)
    {
      size_t totalReceived = 0;

      while(totalReceived < len)
      {
        ssize_t lenReceived = recv(sock, (ubyte_t*)buf+totalReceived, len-totalReceived, 0);

        if(lenReceived < 0)
        {
          throw SocketError(strerror(errno));
        }

        totalReceived += lenReceived;
      }

      return totalReceived;
    }
    else
    {
      ssize_t lenReceived = recv(sock, buf, len, MSG_DONTWAIT);

      if(lenReceived < len)
      {
        if(errno && (errno != EAGAIN))
        {
          throw SocketError(strerror(errno));
        }
      }
      
      if(lenReceived < 0)
      {
        return 0;
      }
      else
      {
        return lenReceived;
      }
    }
  }

  void Socket::Send(const void* const buf, const size_t len)
  {
    if(send(sock, buf, len, MSG_NOSIGNAL) < static_cast<ssize_t>(len))
    {
      throw SocketError(strerror(errno));
    }
  }
}