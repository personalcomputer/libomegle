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

  static const PacketId PID_INIT = "omegleStart";
  static const PacketId PID_INITR1 = "w"; //0x77
  static const PacketId PID_INITR2 = "c"; //0x63
  static const PacketId PID_COUNT = "count";
  static const PacketId PID_TYPING = "t"; //0x74
  static const PacketId PID_STOPTYPING = "st";
  static const PacketId PID_MESSAGE = "s"; //0x73
  static const PacketId PID_STRANGERMESSAGE = "m"; //0x6d
  static const PacketId PID_DISCONNECT = "d"; //0x64
  static const PacketId PID_SUGGESTSPYEE = "suggestSpyee"; //This is useless for us

  struct Packet {};

  struct UserCountPacket : public Packet
  {
    UserCountPacket(const int userCount): userCount(userCount) {}
    int userCount;
  };
  struct StrangerMessagePacket : public Packet
  {
    StrangerMessagePacket(const std::string message): message(message) {}
    std::string message;
  };

  class Connection
  {
    private:
    BufferedSocket sock;

    bool strangerIsTyping;
    int userCount;

    void SendPacket(const PacketId& packetId, const std::string& contents = "");

    public:
    Connection();

    void SendMessage(const std::string message);
    void SendTyping();
    void SendStopTyping();
    void Disconnect();

    int GetUserCount() const;
    bool IsStrangerTyping() const;

    bool PollEvent(PacketId* packetId, Packet** packet, const bool blocking = NONBLOCKING);
    std::string PollMessage(const bool blocking = NONBLOCKING);
  };
}