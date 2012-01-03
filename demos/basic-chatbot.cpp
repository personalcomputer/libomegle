#include <iostream>
#include <string>
#include <cstdlib>

#include "Omegle.h"

// This demo is in the public domain and free to be used for whatever purpose.

/*
 * This is an extremely basic example stateless chat-bot.
 *
 * It starts off the conversation with a 'Hi', and then 
 * can respond to a few select messages with a suitable
 * response, but otherwise resorts to spitting nonsense.
 * (remind you of anyone?)
 */

class BasicBot
{
  private:
  Omegle::Connection& omegleConnection;

  public:
  BasicBot(Omegle::Connection& omegleConnection): omegleConnection(omegleConnection) {}

  void StartConversation()
  {
    // This is called when the bot is first connected with a stranger.

    omegleConnection.SendTyping(); //This will pop up the 'Stranger is typing..' notice to the other stranger
    usleep(800000); //Take almost a second to fake typing
    omegleConnection.SendMessage("Hi");
    omegleConnection.SendStopTyping(); //Note that SendTyping and SendStopTyping are totally unnecessary. They are simply here to create the illusion of being a real person, as if the stranger can't already see through our less than intelligent dialog.
  }

  void RespondToMessage(const std::string& message)
  {
    // This is called when the stranger tells us something
    
    omegleConnection.SendTyping(); //Again - fake typing
    usleep(1500000); //second and a half

    // Decide what to say.
    std::string response;

    // Hardcoded responses:
    if(message == "asl")
    {
      response = "10/robot/space";
    }
    else if(message == "sup")
    {
      response = "Doing the roboto.";
    }
    else
    {
      // Respond randomly from a predefined set of responses
      switch(rand()%10)
      {
        case 0: response = "Ok";
                break;
        case 1: response = "Tell me more";
                break;
        case 2: response = "Haha, how's that?";
                break;
        case 3: response = "Yep";
                break;
        case 4: response = "Really?";
                break;
        case 5: response = "Are you sure?";
                break;
        case 6: response = "I see";
                break;
        case 7: response = "Whatever you say";
                break;
        case 8: response = "That's disgusting!";
                break;
        case 9: response = "Yeah";
                break;
      }
    }

    omegleConnection.SendMessage(response);

    omegleConnection.SendStopTyping();
    
    std::cout << "(Bot): " << response << std::endl; 
  }
};

int main()
{
  srand(time(0));

  try
  {
    Omegle::Connection omegleConnection; //This connects to the Omegle server and will block until Omegle connects us through to a stranger
    std::cerr << "(connected)" << std::endl;

    BasicBot bot(omegleConnection);

    bot.StartConversation();

    while(true)
    {
      std::string message = omegleConnection.PollMessage(Omegle::BLOCKING); //The parameter is optional. BLOCKING means this function will block until a message is received, and NONBLOCKING (default) will not block, ie it will return immediately even if no messages were received (in which case it returns an empty string).

      std::cout << "Stranger: " << message << std::endl; 

      bot.RespondToMessage(message);
    }
  }
  catch(Omegle::ConversationOverError) //Omegle explicitly specifies when the stranger disconnects, although it doesn't work perfectly all the time.
  {
    std::cerr << "(stranger disconnected)" << std::endl;
  }
  catch(Omegle::NetworkError& e)
  {
    std::cerr << "(network failure - " << e.errnoMessage << ")" << std::endl;
  }
  catch(Omegle::CaptchaError)
  {
    std::cerr << "(chatbot foiled by mortal enemy CAPTCHA)" << std::endl;
  }

  return 0;
}
