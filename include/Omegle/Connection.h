#pragma once
#include <string>

#include "Error.h"
#include "BufferedSocket.h"

namespace Omegle
{
  class ConversationOverError : public Error { //For when you poll but the stranger has disconnected.
    public:
    inline ConversationOverError(): Error("conversation over") {}
    inline virtual ~ConversationOverError() throw() {};
  };

  class CaptchaError : public Error { //Omegle employs CAPTCHAs to deter spam. libomegle cannot handle these itself, libomegle is not human. You'll want to capture the returned 'key' value and pass it to reCAPTCHA APIs in order to deal with the CAPTCHA.
    public:
    std::string key;
    inline CaptchaError(const std::string key): Error("captcha required, recaptcha key: "+key), key(key) {}
    inline virtual ~CaptchaError() throw() {};
  };

  typedef std::string PacketId;

  static const PacketId PID_INIT = "omegleStart";
  static const PacketId PID_WAIT = "w"; //0x77
  static const PacketId PID_CONNECTED = "c"; //0x63 //It all seems so simple in retrospect, but it took me a while to realize these were printable characters (I only saw it as id 0x01 0x63).
  static const PacketId PID_USERCOUNT = "count"; //It's somewhat odd that this is sent at all by Omegle
  static const PacketId PID_STARTTYPING = "t"; //0x74
  static const PacketId PID_STOPTYPING = "st";
  static const PacketId PID_MESSAGE = "s"; //0x73
  static const PacketId PID_STRANGERMESSAGE = "m"; //0x6d
  static const PacketId PID_DISCONNECT = "d"; //0x64
  static const PacketId PID_SUGGESTSPYEE = "suggestSpyee"; //This is not exposed. It has pretty much no use for anything but the web client. Being part of a spy conversation, though, may be of interest. Contact me if interested; I know how the protocol works and can whip it up for you.
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
    CaptchaPacket(const std::string key): key(key) {}
    std::string key;
  };

  static const unsigned int SERVER_COUNT = 6;
  typedef unsigned int ServerId_t;

  class Connection
  {
    /* Connection is the concept of a connection to a stranger through Omegle. To establish a connection, merely instantiate a Connection.

    For examples, please see the included demos (particularly demos/basic-chatbot.cpp). They are intended as the primary form of documentation for libomegle.

    ### Usage

    To communicate with the stranger through the connection, call SendMessage(). To Receive their messages, call PollMessage(). libomegle was designed to be very low-friction API-wise.

    You may also be interested in SendTyping() and SendStopTyping(), used by Omegle to indicate your client is actively typing. To check to see if the stranger themselves is typing, check IsStrangerTyping().

    ### Advanced Usage

    ##### Servers

    Omegle operates a few different servers to handle their load. The servers store bans and their CAPTCHA requirements separately for each server. This means if you would like to avoid such bans, you can connect to a different server. To select which server to connect to specify a numeric 'serverId' to Connection's constructor. This id should be in the range [0, SERVER_COUNT).
    
    ##### Raw Packet Handler

    Although PollMessage() is fine for most uses, you may also be interested in handling other events in the conversation more directly (such as when the stranger starts and stops typing). This is possible through PollEvent(), which returns a parsed version of the packets Omegle sends. These packets are available:

    * PID_USERCOUNT
    * PID_STARTTYPING
    * PID_STOPTYPING
    * PID_STRANGERMESSAGE
    * PID_DISCONNECT
    * PID_CAPTCHA

    PID_USERCOUNT, PID_STRANGERMESSAGE, and PID_CAPTCHA have additional data associated with them. You can access this data through the `packet` parameter, and then cast it to either a UserCountPacket, StrangerMessagePacket, or CaptchaPacket (respectively).
    */
    private:
    BufferedSocket sock;

    bool strangerIsTyping;
    int userCount;

    void SendPacket(const PacketId& packetId, const std::string& contents = "");
    
    public:
    Connection(const ServerId_t serverId = 0);
    ~Connection();

    void SendMessage(const std::string message);
    void SendTyping();
    void SendStopTyping();
    void Disconnect();

    int GetUserCount() const; //Although I can't imagine this being very useful, if you have a full Omegle client, you may wish to expose this. If you would like to just track statistics or something though, I recommend using the web interface for acquiring the usercount as it requires no stranger connection.
    bool IsStrangerTyping() const;

    bool PollEvent(PacketId* const packetId, Packet** const packet, const bool blocking = NONBLOCKING); //packetId and packet are output. //In nonblocking mode, true is returned if there is an event to handle, and false otherwise.
    std::string PollMessage(const bool blocking = NONBLOCKING);
  };
}