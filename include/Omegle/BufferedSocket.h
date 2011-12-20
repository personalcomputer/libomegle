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
  /* BufferedSocket is an encapsulation of a nonblocking TCP socket. The core idea is that it buffers all data that passes through it, to give you freedom to build complete junks of data to send at once (application-level packets), and to give you freedom to wait for one of these application packets to fully assemble itself locally. It will only flush the outgoing buffer when you tell it and will only flush the incoming buffer when you are satisfied with what you've received
  

  #### Sending data with BufferedSocket

  Sending data with BufferedSocket is just like sending with `send()`, except with one more step.

  First, tell it to send data with `QueueSend()` using the same parameters as `send()`. This is called QueueSend for a reason though, it hasn't actually sent any yet. To actually send the queued data, call `FlushSend()`. You may Queue up as much data as you want before calling `FlushSend()`
  

  #### Receiving data with BufferedSocket

  Receiving data with a BufferedSocket is a little bit more complicated. Although BufferedSockets can be made to act like normal sockets, to see their real usefulness I'm not going to introduce them from that perspective.

  When you receive data in most applications, you are not really looking to receiving an unusable incomplete chunk of data, you are looking to receiving the required minimal amount of data that it takes to parse it into something useful. For example, this may be a complete instant message in an IRC protocol, or a complete packet instructing a spaceship where to move in a multi-player game. The obvious solution to this is to simply block until you have received that much data, but this is impractical in a lot of situations such as the aforementioned realtime multiplayer game, where lag needs to be hidden as much as possible. Thus I created BufferedSocket to allow you to operate on complete chunks of data without blocking execution. This is accomplished by keeping a buffer containing received, but unhanded, data. You then *attempt* to parse this data every so often, until all the required parts make it safely through the network, at which point you 'accept' the just parsed data and it is cleared from the BufferedSocket's buffer. To do this process, simply call `CheckRecvBuffer()`. This returns two values, the size of the socket's buffer, and a pointer to the buffer. At this point, attempt to parse the provided buffer (don't go over the size of it though). If you successfully parse a chunk of data from it, call `AcceptAndClearRecvBuffer()` with the length of the data successfully parsed. If when parsing you find there is not enough data to form a complete parsable chunk, then no problem, just continue along without accepting any of it and it will be waiting for you to attempt to parse again when more data has come down the pipe.

  In addition, BufferedSockets can optionally be made to block when receiving data by calling `WaitRecvBuffer()`, which will block until the socket buffer is at least as large as the request size. If you always want to block when receiving data though, I recommend using raw sockets, although BufferedSocket still will provide some use.
  */
  
  {
    private:
    int sock;

    void* recvBuffer;
    size_t recvBufferLen; //This is not the size of the above, but instead the in-use size of the above. The total size is SOCKET_MAXBUFFSIZE.

    void* sendQueue;
    size_t sendQueueLen; //This is not the size of the above, but instead the in-use size of the above. The total size is SOCKET_MAXBUFFSIZE.

    bool blocking;

    void RecvIntoBuffer(const size_t requiredLen = 0);

    public:
    BufferedSocket();
    ~BufferedSocket();

    void Connect(const std::string& address, const std::string& port);
    void Disconnect();

    bool IsConnected();

    void QueueSend(const void* const data, const size_t dataLen);
    void FlushSendQueue();
    size_t GetSendQueueLength();

    const void* CheckRecvBuffer(size_t* const bufferLen);
    void WaitRecvBuffer(const size_t requiredBufferLen); //Note that this does not return anything. It simply waits.
    void AcceptAndClearRecvBuffer(const size_t approvedLen);
  };
}