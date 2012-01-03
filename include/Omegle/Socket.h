#pragma once
#include <string>
#include <cerrno>
#include <cstring>

#include "Error.h"

namespace Omegle
{
  class NetworkError : public Error {
    public:
    const int errsv; //(presumably recommended) naming from the errno manpage.
    const std::string errnoMessage;
    inline NetworkError(const int errsv): Error(std::string("network error: ")+strerror(errsv)), errsv(errsv), errnoMessage(strerror(errsv)) {}
    inline virtual ~NetworkError() throw() {}
  };

  static const bool BLOCKING = true;
  static const bool NONBLOCKING = false;

  typedef unsigned char ubyte_t;

  class Socket
  /* Socket is nothing more than an encapsulation of basic socket functionality with a little less C and little more C++. */
  {
    private:
    int sock;

    public:
    virtual ~Socket();

    void Connect(const std::string& address, const std::string& port);
    void Disconnect();

    bool IsConnected() const;

    size_t Recv(void* const buf, const size_t len, const bool blocking = BLOCKING); //returns the length of the received data. if it is less than the amount you wanted to received and you specified NONBLOCKING, then you'll just have to wait some more. Otherwise, it will return the amount you wanted to receive.
    void Send(const void* const buf, const size_t len);
  };
}