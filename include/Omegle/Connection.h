#pragma once
#include <string>

#include "Error.h"
#include "BufferedSocket.h"

namespace Omegle
{
  static const bool BLOCKING = true;
  static const bool NONBLOCKING = false;

  class ConversationOverError : public Error {
    public:
    inline ConversationOverError(const std::string message): Error(message) {}
    inline ConversationOverError(): Error() {}
  };

  typedef std::string PacketId;

  class Connection
  {
    private:
    BufferedSocket sock;

    bool strangerIsTyping;
    int userCount;

    void SendPacket(const PacketId& packetId, const std::string& contents = "");

    bool PollIncommingPackets(PacketId* const packetId, std::string* const contents, const bool blocking = NONBLOCKING);

    public:
    Connection();

    void SendMessage(const std::string message);
    void SendTyping();
    void SendStopTyping();
    void Disconnect();

    int GetUserCount() const;
    bool IsStrangerTyping() const;

    std::string PollMessage(const bool blocking = NONBLOCKING);
  };
}