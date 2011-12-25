#pragma once
#include <string>
#include <exception>

namespace Omegle
{
  class Error : public std::exception
  {
    private:
    std::string m_message;
    
    public:
    inline Error(std::string message): m_message(message) {}
    inline Error(): m_message("generic error") {}
    
    inline virtual ~Error() throw() {};
    
    inline virtual const char* what() const throw()
    {
      return m_message.c_str();
    }
  };
}