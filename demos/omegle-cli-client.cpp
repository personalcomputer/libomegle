#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>

#include "Omegle.h"

void inputThreadMain(Omegle::Connection* omegleConnection)
{
  while(true)
  {
    char message[257];
    std::cin.getline(message, 256, '\n');

    omegleConnection->SendMessage(message);
    std::cout << "You: " << message << std::endl; 
  }
}

int main()
{
  Omegle::Connection omegleConnection;
  std::cerr << "You're now chatting with a random stranger. Say hi!" << std::endl;

  std::thread inputThread(inputThreadMain, &omegleConnection);

  while(true)
  {
    usleep(100000); //prevent CPU rape //there is no blocking mode at the moment

    try
    {
      std::string response = omegleConnection.PollMessage();

      if(response != "")
      {
        std::cout << "Stranger: " << response << std::endl; 
      }
    }
    catch(Omegle::ConversationOverError)
    {
      std::cerr << "Conversational partner has disconnected." << std::endl;
      exit(0);
    }
    catch(Omegle::SocketError)
    {
      std::cerr << "Network failure." << std::endl;
      exit(0);
    }
  }
}
