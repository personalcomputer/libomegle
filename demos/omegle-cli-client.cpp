#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "Omegle.h"

// This demo is in the public domain and free to be used for whatever purpose.

// See basic-chatbot.cpp for a well-commented introductory use of the more basic API. This demo demos the more advanced API and handling all events. 

void inputThreadMain(Omegle::Connection* omegleConnection)
{
  while(true)
  {
    std::string message;
      
    char c = std::cin.get();
    message += c;

    omegleConnection->SendTyping();

    while((c = std::cin.get()) != '\n')
    {
      message += c;
    }

    if(message == "\\quit")
    {
      std::cerr << "(You have disconnected)" << std::endl;
      omegleConnection->Disconnect();
      exit(0);
    }

    omegleConnection->SendMessage(message);
    std::cout << "You: " << message << std::endl; 
  }
}

int main()
{
  Omegle::Connection omegleConnection;
  std::cerr << "You're now chatting with a random stranger. Say hi!" << std::endl;

  std::thread inputThread(inputThreadMain, &omegleConnection);

  try
  {
    while(true)
    {
      Omegle::PacketId packetId;
      Omegle::Packet* packet;

      omegleConnection.PollEvent(&packetId, &packet, Omegle::BLOCKING);

      if(packetId == Omegle::PID_STRANGERMESSAGE)
      {
        std::cout << "Stranger: " << static_cast<Omegle::StrangerMessagePacket*>(packet)->message << std::endl;
      }
      else if(packetId == Omegle::PID_TYPING)
      {
        std::cerr << "(Stranger is typing)" << std::endl;
      }
      else if(packetId == Omegle::PID_STOPTYPING)
      {
        std::cerr << "(Stranger has stopped typing)" << std::endl;
      }
      else if(packetId == Omegle::PID_DISCONNECT)
      {
        std::cerr << "Conversational partner has disconnected." << std::endl;
        exit(0);
      }
    }
  }
  catch(Omegle::SocketError)
  {
    std::cerr << "Network failure." << std::endl;
    exit(1);
  }
}
