//============================================================================
// Name        : Rekt.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Rekt.h"

#include <iostream>
#include "Lexer.h"
#include "Opt.h"
#include "Parser.h"
#include "CodeGenerator.h"

using namespace std;

int main(int argc, char **argv)
{
  int ret = 0;

  // 1) Process Options
  Opt opt; if (!ret) ret = opt.Process(argc, argv);

  // 2) Lexer
  Lexer lexer; if (!ret) ret = lexer.Process(opt);

  // 3) Parser
  Parser parser; if (!ret) ret = parser.Process(lexer);

  // 4) CodeGenerator
  CodeGen cg; if (!ret) ret = cg.Process(opt, parser);

  if (ret)
  {
    cout << "Failed in ";
    if (ret >= 50)
      cout << "CODEGEN ";
    else if (ret >= 40)
      cout << "OPTIMIZER ";
    else if (ret >= 30)
      cout << "PARSER ";
    else if (ret >= 20)
      cout << "LEXER ";
    else if (ret >= 10)
      cout << "OPTION ";
    else
      cout << "UNKNOWN ";
    cout << "with return " << ret << endl;
    return ret;
  }
  cout << "\nSuccess!" << endl;
  return 0;
}
