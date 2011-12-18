#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cassert>
#include <string>

#include "Error.h"

#include "BufferedSocket.h"

namespace Omegle
{
  static const size_t SOCKET_MAXBUFFSIZE = 5120; //5KB //can easily make this a soft limit, but it is a whole lot simpler just allocating for the max case.

  BufferedSocket::BufferedSocket(const std::string& address, const std::string& port): recvBufferLen(0), sendQueueLen(0)
  {
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

    fcntl(sock, F_SETFL, O_NONBLOCK);

    recvBuffer = malloc(SOCKET_MAXBUFFSIZE);
    sendQueue = malloc(SOCKET_MAXBUFFSIZE);
  }

  BufferedSocket::~BufferedSocket()
  {
    close(sock);

    free(recvBuffer);
    free(sendQueue);
  }

  void BufferedSocket::QueueSend(const void* const data, const size_t dataLen)
  {
    assert(sendQueueLen + dataLen <= SOCKET_MAXBUFFSIZE);

    memcpy(sendQueue+sendQueueLen, data, dataLen);
    sendQueueLen += dataLen;
  }

  void BufferedSocket::FlushSendQueue()
  {
    if(send(sock, sendQueue, sendQueueLen, 0) < static_cast<ssize_t>(sendQueueLen))
    {
      throw SocketError(strerror(errno));
    }

    sendQueueLen = 0;
  }

  size_t BufferedSocket::GetSendQueueLength()
  {
    return sendQueueLen;
  }

  void BufferedSocket::RecvIntoBuffer()
  {
    int lenRecieved = recv(sock, recvBuffer+recvBufferLen, SOCKET_MAXBUFFSIZE-recvBufferLen, 0);
    if(lenRecieved < static_cast<int>(SOCKET_MAXBUFFSIZE-recvBufferLen))
    {
      if(errno == EAGAIN || errno == 0)
      {
        if(lenRecieved >= 1)
        {
          recvBufferLen += lenRecieved;
        }
        return;
      }
      else
      {
        throw SocketError(strerror(errno));
      }
    }
    else
    {
      throw SocketError("Socket receiving buffer overflow."); //Yes - this means you must keep packets to SOCKET_MAXBUFFSIZE-1 bytes and handle them (approve&clear) before they build up.
    }
  }

  void* BufferedSocket::CheckRecvBuffer(size_t* const bufferLen)
  {
    RecvIntoBuffer();

    *bufferLen = recvBufferLen;
    return recvBuffer;
  }

  void BufferedSocket::AcceptAndClearRecvBuffer(const size_t approvedLen)
  {
    recvBufferLen -= approvedLen;

    memmove(recvBuffer, recvBuffer+approvedLen, recvBufferLen); //Pretty certain memcpy will have no problems doing this as well, despite the overlap, but I hardly need that performance.
  }
}