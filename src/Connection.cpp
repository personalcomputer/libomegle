#include <string>
#include <cassert>
#include <cstdlib>
#include <arpa/inet.h>
#include <time.h>

#include "Error.h"
#include "BufferedSocket.h"

#include "Connection.h"

namespace Omegle
{
  typedef unsigned char byte_t;

  static const std::string SERVERS[2] = {"bajor.omegle.com", "cardassia.omegle.com"};
  static const std::string PORT = "1365";

  static const int INIT_PACKET_ABTEST_LENGTH = 142;
  static const std::string INIT_PACKET_STRING = "web-flash?rcs=1&spid=&abtest=";

  Connection::Connection(): sock(SERVERS[0], PORT), strangerIsTyping(false), userCount(0)
  {
    // Generate a meaningless value for omegle to track us with.
    srand(time(0));

    std::string abtest(INIT_PACKET_ABTEST_LENGTH, 'a');
    for(int i = 0; i < INIT_PACKET_ABTEST_LENGTH; ++i)
    {
      char c = (rand()%(26+10))+(97-10); //Generate a random lowercase letter, although it may be up to ten characters off. These will be converted to numbers.

      if(c < 97)
      {
        c -= 39;
      }

      abtest[i] = c;
    }

    // Do the little init sequence.
    SendPacket(PID_INIT, INIT_PACKET_STRING+abtest);

    PacketId packetId;
    Packet* packet;

    PollEvent(&packetId, &packet, BLOCKING);
    if(packetId != PID_INITR2)
    {
      PollEvent(&packetId, &packet, BLOCKING);
      assert(packetId == PID_INITR2);
    }

    //std::cerr << "(debug) You're now chatting with a random stranger. Say hi!" << std::endl;
  }

  void Connection::SendPacket(const PacketId& id, const std::string& contents)
  {
    // Send ID
    byte_t idLen = id.length();
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
    SendPacket(PID_TYPING);
  }

  void Connection::SendStopTyping()
  {
    SendPacket(PID_STOPTYPING);
  }

  void Connection::Disconnect()
  {
    SendPacket(PID_DISCONNECT);
  }

  int Connection::GetUserCount() const
  {
    return userCount;
  }

  bool Connection::IsStrangerTyping() const
  {
    return strangerIsTyping;
  }

  bool Connection::PollEvent(PacketId* packetId, Packet** packet, const bool blocking)
  {
    size_t acceptedBufferLen = 0;
    size_t bufferLen;
    const void* buffer;

    if(!blocking)
    {
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }

    // Receive ID
    byte_t idLen;

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + sizeof(idLen));
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen < sizeof(idLen))
    {
      return false;
    }

    idLen = *reinterpret_cast<const byte_t*>(buffer+acceptedBufferLen);
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

    *packetId = std::string(reinterpret_cast<const char*>(buffer+acceptedBufferLen), idLen);
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

    contentsLen = *reinterpret_cast<const short*>(buffer+acceptedBufferLen);
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
    
    contents = std::string(reinterpret_cast<const char*>(buffer+acceptedBufferLen), contentsLen);
    acceptedBufferLen += contentsLen;

    sock.AcceptAndClearRecvBuffer(acceptedBufferLen);

    if(!(*packetId == PID_INIT ||
         *packetId == PID_INITR1 ||
         *packetId == PID_INITR2 ||
         *packetId == PID_COUNT ||
         *packetId == PID_TYPING ||
         *packetId == PID_STOPTYPING ||
         *packetId == PID_MESSAGE ||
         *packetId == PID_STRANGERMESSAGE ||
         *packetId == PID_DISCONNECT ||
         *packetId == PID_SUGGESTSPYEE))
    {
      throw Error("Unknown packet type received (" + *packetId + ((contents != "")? ")" : (", " + contents + ")")) + ".");
    }

    // Handle packets based on type, parsing contents into appropriate Packet datastructure, and/or set internal get-able state (strangerIsTyping, userCount)
    *packet = NULL;
    if(*packetId == PID_STRANGERMESSAGE)
    {
      *packet = new StrangerMessagePacket(contents);
    }
    else if(*packetId == PID_COUNT)
    {
      *packet = new UserCountPacket(atoi(contents.c_str()));
      userCount = ((UserCountPacket*)packet)->userCount;
    }
    else if(*packetId == PID_TYPING)
    {
      strangerIsTyping = true;
    }
    else if(*packetId == PID_STOPTYPING)
    {
      strangerIsTyping = false;
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
        throw ConversationOverError();
      }
    }

    return "";
  }
}