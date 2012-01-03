#include <string>
#include <cassert>
#include <cstdlib>
#include <arpa/inet.h>
#include <time.h>
#include <iostream>

#include "Error.h"
#include "BufferedSocket.h"

#include "Connection.h"

namespace Omegle
{
  static const std::string SERVERS[SERVER_COUNT] = {"bajor.omegle.com", "promenade.omegle.com", "cardassia.omegle.com", "quarks.omegle.com"};
  static const std::string PORT = "1365";

  static const int INIT_PACKET_ABTEST_LENGTH = 142;
  static const std::string INIT_PACKET_STRING = "web-flash?rcs=1&spid=&abtest=";

  static const int CHARACTERS_IN_ALPHABET = 26;


  char RandomAlphanumericChar()
  {
    char c = (rand()%(CHARACTERS_IN_ALPHABET+10))+('a'-10); //Generate a random lowercase letter, although it may be up to ten characters off. These will be converted to numbers.

    if(c < 'a') //If it is one of the off-characters (not a letter)
    {
      c -= ('a'-1) - '9'; //move in into the range of numbers
    }

    return c;
  }

  Connection::Connection(const ServerId_t serverId): strangerIsTyping(false), userCount(0)
  {
    // Generate a meaningless value for omegle to track us with.
    srand(time(0));

    std::string abtest(INIT_PACKET_ABTEST_LENGTH, 'a');
    for(int i = 0; i < INIT_PACKET_ABTEST_LENGTH; ++i)
    {
      abtest[i] = RandomAlphanumericChar();
    }


    sock.Connect(SERVERS[serverId], PORT);

    // Do the little init sequence.
    SendPacket(PID_INIT, INIT_PACKET_STRING+abtest);

    PacketId packetId;
    Packet* packet;
    while(PollEvent(&packetId, &packet, BLOCKING) && (packetId != PID_CONNECTED))
    {
      if(packetId == PID_WAIT)
      {
        continue;
      }
      else if(packetId == PID_CAPTCHA)
      {
        throw CaptchaError(((CaptchaPacket*)packet)->key);
      }
      else
      {
        throw Error("Unexpected packet received while establishing connection (\""+packetId+"\")");
      }
    }
  }

  Connection::~Connection()
  {
    sock.Disconnect();
  }

  void Connection::SendPacket(const PacketId& id, const std::string& contents)
  {
    // Send ID
    ubyte_t idLen = id.length();
    sock.QueueSend(&idLen, sizeof(idLen));
    sock.QueueSend(id.data(), idLen);

    // Send contents
    unsigned short contentsLen = contents.length();
    contentsLen = htons(contentsLen);
    sock.QueueSend(&contentsLen, sizeof(contentsLen));
    contentsLen = ntohs(contentsLen);
    sock.QueueSend(contents.data(), contentsLen);

    sock.FlushSendQueue();
  }

  void Connection::SendMessage(const std::string message)
  {
    SendPacket(PID_MESSAGE, message);
  }

  void Connection::SendTyping()
  {
    SendPacket(PID_STARTTYPING);
  }

  void Connection::SendStopTyping()
  {
    SendPacket(PID_STOPTYPING);
  }

  void Connection::Disconnect()
  {
    try
    {
      SendPacket(PID_DISCONNECT);
    }
    catch(NetworkError) {}

    sock.Disconnect();
  }

  int Connection::GetUserCount() const
  {
    return userCount;
  }

  bool Connection::IsStrangerTyping() const
  {
    return strangerIsTyping;
  }

  bool Connection::PollEvent(PacketId* const packetId, Packet** const packet, const bool blocking)
  {
    assert(packetId && packet);

    size_t acceptedBufferLen = 0;
    size_t bufferLen;
    const void* buffer;

    if(!blocking)
    {
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }

    // Receive ID
    ubyte_t idLen;

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + sizeof(idLen));
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen < sizeof(idLen))
    {
      return false;
    }

    idLen = *reinterpret_cast<const ubyte_t*>((ubyte_t*)buffer+acceptedBufferLen);
    assert(idLen >= 1);
    acceptedBufferLen += sizeof(idLen);

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + idLen);
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen-acceptedBufferLen < idLen)
    {
      return false;
    }

    *packetId = std::string(reinterpret_cast<const char*>((ubyte_t*)buffer+acceptedBufferLen), idLen);
    acceptedBufferLen += idLen;
    
    // Receive contents
    unsigned short contentsLen;

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + sizeof(contentsLen));
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen < sizeof(contentsLen))
    {
      return false;
    }

    contentsLen = *reinterpret_cast<const short*>((ubyte_t*)buffer+acceptedBufferLen);
    contentsLen = ntohs(contentsLen);
    acceptedBufferLen += sizeof(contentsLen);

    std::string contents;

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + contentsLen);
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen-acceptedBufferLen < contentsLen)
    {
      return false;
    }
    
    contents = std::string(reinterpret_cast<const char*>((ubyte_t*)buffer+acceptedBufferLen), contentsLen);
    acceptedBufferLen += contentsLen;

    sock.AcceptAndClearRecvBuffer(acceptedBufferLen);

    if(!(*packetId == PID_INIT ||
         *packetId == PID_WAIT ||
         *packetId == PID_CONNECTED ||
         *packetId == PID_USERCOUNT ||
         *packetId == PID_STARTTYPING ||
         *packetId == PID_STOPTYPING ||
         *packetId == PID_MESSAGE ||
         *packetId == PID_STRANGERMESSAGE ||
         *packetId == PID_DISCONNECT ||
         *packetId == PID_SUGGESTSPYEE ||
         *packetId == PID_CAPTCHA))
    {
      throw Error("Unknown packet type received (" + *packetId + ((contents != "")? ")" : (", " + contents + ")")) + ".");
    }

    // Handle packets based on type, parsing contents into appropriate Packet datastructure, set internal request-able state (strangerIsTyping, userCount), and in the case of PID_DISCONNECT, closing the underlying socket.
    *packet = NULL;
    if(*packetId == PID_STRANGERMESSAGE)
    {
      *packet = new StrangerMessagePacket(contents);
    }
    else if(*packetId == PID_USERCOUNT)
    {
      *packet = new UserCountPacket(atoi(contents.c_str()));
      userCount = ((UserCountPacket*)packet)->userCount;
    }
    else if(*packetId == PID_CAPTCHA)
    {
      size_t delim = contents.find_first_of("=");
      assert(delim != std::string::npos);
      delim++;
      assert(contents.substr(0,delim) == "publicKey=");

      std::string key = contents.substr(delim, contents.length()-delim);

      *packet = new CaptchaPacket(key);
    }
    else if(*packetId == PID_STARTTYPING)
    {
      strangerIsTyping = true;
    }
    else if(*packetId == PID_STOPTYPING)
    {
      strangerIsTyping = false;
    }
    else if(*packetId == PID_DISCONNECT)
    {
      sock.Disconnect();
    }

    return true;
  }

  std::string Connection::PollMessage(const bool blocking)
  {
    PacketId packetId;
    Packet* packet;

    while(PollEvent(&packetId, &packet, blocking))
    {
      if(packetId == PID_STRANGERMESSAGE)
      {
        return static_cast<StrangerMessagePacket*>(packet)->message;
      }
      else if(packetId == PID_DISCONNECT)
      {
        throw ConversationOverError(); //Not quite the most elegant use of exceptions.
      }
    }

    return "";
  }
}
