#include <cstring>
#include <cstdlib>
#include <cassert>
#include <string>

#include "Error.h"

#include "BufferedSocket.h"

namespace Omegle
{
  static const size_t SOCKET_MAXBUFFSIZE = 5120; //5KB //can easily make this a soft limit, but it is a whole lot simpler just allocating for the max case.

  BufferedSocket::BufferedSocket(): recvBufferLen(0), sendQueueLen(0)
  {
    recvBuffer = malloc(SOCKET_MAXBUFFSIZE);
    sendQueue = malloc(SOCKET_MAXBUFFSIZE);
  }

  BufferedSocket::~BufferedSocket()
  {
    Disconnect();

    free(recvBuffer);
    free(sendQueue);
  }

  void BufferedSocket::QueueSend(const void* const data, const size_t dataLen)
  {
    assert(sendQueueLen + dataLen <= SOCKET_MAXBUFFSIZE);

    memcpy((ubyte_t*)sendQueue+sendQueueLen, data, dataLen);
    sendQueueLen += dataLen;
  }

  void BufferedSocket::FlushSendQueue()
  {
    Send(sendQueue, sendQueueLen);
    sendQueueLen = 0;
  }

  size_t BufferedSocket::GetSendQueueLength()
  {
    return sendQueueLen;
  }

  void BufferedSocket::RecvIntoBuffer(const size_t requiredBufferLen)
  {
    if((requiredBufferLen != 0) && (recvBufferLen >= requiredBufferLen))
    {
      return;
    }

    recvBufferLen += Recv((ubyte_t*)recvBuffer+recvBufferLen, (requiredBufferLen == 0)? SOCKET_MAXBUFFSIZE-recvBufferLen : requiredBufferLen-recvBufferLen, (requiredBufferLen == 0)? NONBLOCKING : BLOCKING);

    assert(recvBufferLen >= requiredBufferLen);
  }

  const void* BufferedSocket::CheckRecvBuffer(size_t* const bufferLen)
  {
    RecvIntoBuffer();

    *bufferLen = recvBufferLen;
    return recvBuffer;
  }

  void BufferedSocket::WaitRecvBuffer(const size_t requiredBufferLen)
  {
    RecvIntoBuffer(requiredBufferLen);
  }

  void BufferedSocket::AcceptAndClearRecvBuffer(const size_t approvedLen)
  {
    recvBufferLen -= approvedLen;

    memmove(recvBuffer, (ubyte_t*)recvBuffer+approvedLen, recvBufferLen); //Pretty certain memcpy will have no problems doing this as well, despite the overlap, but I hardly need that performance.
  }
}