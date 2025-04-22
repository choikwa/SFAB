#include "Lexeme.h"

namespace Lexeme
{
bool isKeyword(int id)
{
  switch(id)
  {
  case IF:
  case ELSE:
  case FOR:
  case WHILE:
  case SWITCH:
  case ALL:
  case NONE:
  case OVERLOAD:
  case RETURN:
  case SET:
    return true;
  }
  return false;
}
}
