#pragma once
#include <string>

#include "Error.h"
#include "BufferedSocket.h"

namespace Omegle
{
  class ConversationOverError : public Error {
    public:
    inline ConversationOverError(): Error("conversation over") {}
    inline virtual ~ConversationOverError() throw() {};
  };

  class CaptchaError : public Error {
    public:
    std::string key;
    inline CaptchaError(const std::string key): Error("captcha required, recaptcha key: "+key), key(key) {}
    inline virtual ~CaptchaError() throw() {};
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
  static const PacketId PID_CAPTCHA = "recaptchaRequired";

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
  struct CaptchaPacket : public Packet
  {
    CaptchaPacket(const std::string key): key(key) {} //To use this key for all intents and purposes you'll have to use the recaptcha API. Like, http://api.recaptcha.net/js/recaptcha_ajax.js
    std::string key;
  };

  static const int SERVER_COUNT = 3;

  class Connection
  {
    private:
    BufferedSocket sock;

    bool strangerIsTyping;
    int userCount;

    void SendPacket(const PacketId& packetId, const std::string& contents = "");

    public:
    Connection(const int serverId = 0);
    ~Connection();

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