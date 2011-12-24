#pragma once
#include <string>
#include <stdexcept>

namespace Omegle
{
  class Error : public std::runtime_error
  {
    public:
    inline Error(const std::string message): std::runtime_error(message) {}
    inline Error(): std::runtime_error("(no message)") {}
  };
}