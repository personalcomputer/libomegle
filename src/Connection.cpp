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

  static const std::string SERVER = "bajor.omegle.com";
  static const std::string PORT = "1365";

  static const int INIT_PACKET_ABTEST_LENGTH = 142;
  static const std::string INIT_PACKET_STRING = "web-flash?rcs=1&spid=&abtest=";

  static const PacketId PID_INIT("omegleStart");
  static const PacketId PID_INITR1("w"); //0x77
  static const PacketId PID_INITR2("c"); //0x63
  static const PacketId PID_COUNT("count");
  static const PacketId PID_TYPING("t"); //0x74
  static const PacketId PID_STOPTYPING("st");
  static const PacketId PID_MESSAGE("s"); //0x73
  static const PacketId PID_STRANGERMESSAGE("m"); //0x6d
  static const PacketId PID_DISCONNECT("d"); //0x64
  static const PacketId PID_SUGGESTSPYEE("suggestSpyee"); //This is useless for us

  Connection::Connection(): sock(SERVER, PORT), strangerIsTyping(false), userCount(0)
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
    std::string contents;

    PollIncommingPackets(&packetId, &contents, BLOCKING);
    if(packetId != PID_INITR2)
    {
      PollIncommingPackets(&packetId, &contents, BLOCKING);
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

  bool Connection::PollIncommingPackets(PacketId* packetId, std::string* contents, const bool blocking)
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

    if(blocking)
    {
      sock.WaitRecvBuffer(acceptedBufferLen + contentsLen);
      buffer = sock.CheckRecvBuffer(&bufferLen);
    }
    else if(bufferLen-acceptedBufferLen < contentsLen)
    {
      return false;
    }
    
    *contents = std::string(reinterpret_cast<const char*>(buffer+acceptedBufferLen), contentsLen);
    acceptedBufferLen += contentsLen;

    sock.AcceptAndClearRecvBuffer(acceptedBufferLen);

    return true;
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

  std::string Connection::PollMessage(const bool blocking)
  {
    PacketId packetId;
    std::string contents;

    while(PollIncommingPackets(&packetId, &contents, blocking))
    {
      if(packetId == PID_STRANGERMESSAGE)
      {
        return contents;
      }
      else if(packetId == PID_COUNT)
      {
        //std::cerr << "(debug) user count: " << contents << std::endl;
        userCount = atoi(contents.c_str());
      }
      else if(packetId == PID_TYPING)
      {
        //std::cerr << "(debug) stranger is typing" << std::endl;
        strangerIsTyping = true;
      }
      else if(packetId == PID_STOPTYPING)
      {
        //std::cerr << "(debug) stranger stopped typing" << std::endl;
        strangerIsTyping = false;
      }
      else if(packetId == PID_DISCONNECT)
      {
        throw ConversationOverError();
      }
      else if(packetId == PID_SUGGESTSPYEE)
      {
        //useless to us
      }
      else
      {
        throw Error("Unknown packet type received (" + packetId + ((contents != "")? ")" : (", " + contents + ")")) + ".");
      }
    }  

    return "";
  }
}