#pragma once
#include <string>

#include "Error.h"

namespace Omegle
{
  class SocketError : public Error {
    public:
    inline SocketError(const std::string message): Error(message) {}
    inline SocketError(): Error() {}
  };

  class BufferedSocket
  /* BufferedSocket is an encapsulation of a nonblocking TCP socket. The core idea is that it buffers all data that passes through it, to give you freedom to build complete junks of data to send at once (application-level packets), and to give you freedom to wait for one of these application packets to fully assemble itself locally. It will only flush the outgoing buffer when you tell it and will only flush the incoming buffer when you are satisfied with what you've received. */
  {
    private:
    int sock;

    void* recvBuffer;
    size_t recvBufferLen; //This is not the size of the above, but instead the in-use size of the above. The total size is SOCKET_MAXBUFFSIZE.

    void* sendQueue;
    size_t sendQueueLen; //This is not the size of the above, but instead the in-use size of the above. The total size is SOCKET_MAXBUFFSIZE.

    bool blocking;

    void RecvIntoBuffer();

    public:
    BufferedSocket(const std::string& address, const std::string& port);
    ~BufferedSocket();

    void QueueSend(const void* const data, const size_t dataLen);
    void FlushSendQueue();
    size_t GetSendQueueLength();

    void* CheckRecvBuffer(size_t* const bufferLen);
    void AcceptAndClearRecvBuffer(const size_t approvedLen);
  };
}