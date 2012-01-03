#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

#include "Omegle.h"

// This demo is in the public domain and free to be used for whatever purpose.

// See basic-chatbot.cpp for a well-commented introductory use of the more basic API.

// This ended up quite a bit longer than it should be because of the difficulties in asynchronous terminal input/output.

int main()
{
  try
  {
    Omegle::Connection omegleConnection;
    std::cerr << "You're now chatting with a random stranger. Say hi!" << std::endl;

    std::string inputBuffer;

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    while(true)
    {
      // I should just select() on stdin and the omegleConnection instead of relying on two nonblocking calls, but this is easier said than done because the raw FD is not exposed by libomegle.
      usleep(50000); //Prevent spinlock from eating CPU

      // Handle input
      char input[256];

      ssize_t stdinLenRead = read(STDIN_FILENO, input, 256);
      if(stdinLenRead > 0)
      {
        if(inputBuffer == "")
        {
          omegleConnection.SendTyping();
        }
          
        inputBuffer.append(input, stdinLenRead);

        unsigned int newlinePos;
        while((newlinePos = inputBuffer.find('\n')) != std::string::npos) //While complete messages have been entered by the user
        {
          std::string message = inputBuffer.substr(0, newlinePos); //Split the message from excess input
          inputBuffer = inputBuffer.substr(newlinePos+1, inputBuffer.length()); //Leave whatever excess you have typed in the input buffer to be handled next iteration

          if(message == "\\quit") //CTRL+D works too
          {
            std::cerr << "(You have disconnected)" << std::endl;
            omegleConnection.Disconnect();
            exit(0);
          }

          omegleConnection.SendMessage(message);
          std::cout << "You: " << message << std::endl;

          if(!inputBuffer.empty())
          {
            omegleConnection.SendTyping();
          }
        }
      }
      else if(stdinLenRead < 0)
      {
        assert(errno == EAGAIN);
      }
      else //stdinLenRead = 0 //EOF //CTRL+D
      {
        std::cerr << "(You have disconnected)" << std::endl;
        omegleConnection.Disconnect();
        exit(0);
      }

      // Handle packets
      Omegle::PacketId packetId;
      Omegle::Packet* packet;

      omegleConnection.PollEvent(&packetId, &packet, Omegle::NONBLOCKING);

      if(packetId == Omegle::PID_STRANGERMESSAGE)
      {
        std::cout << "Stranger: " << static_cast<Omegle::StrangerMessagePacket*>(packet)->message << std::endl;
      }
      else if(packetId == Omegle::PID_STARTTYPING)
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
  catch(Omegle::NetworkError& e)
  {
    std::cerr << "(network failure: " << e.errnoMessage << ")" << std::endl;
    exit(1);
  }
  catch(Omegle::CaptchaError)
  {
    std::cerr << "CAPTCHA required. (It simply isn't practical or really possible to get the CAPTCHA and display it in this CLI environment. Therefore, this is fatal until resolved elsewhere.)" << std::endl;
    exit(1);
  }
}
