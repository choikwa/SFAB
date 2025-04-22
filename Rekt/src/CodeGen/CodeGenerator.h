/*
 * CodeGenerator.h
 *
 *  Created on: Nov 2, 2015
 *      Author: flamedoge
 */

#ifndef CODEGENERATOR_H_
#define CODEGENERATOR_H_

#include <string>

class Opt;
class Parser;
namespace AST
{
class Node;
}
using namespace std;
using namespace AST;
namespace CodeGenWS
{
void defaultEval(Node *n);
void keywordEval(const char *c, Node *n);
void evaluate(Node *n);
}
class CodeGen
{
public:
  int Process(Opt &opt, Parser &paser);
  std::string prog;
private:

};



#endif /* CODEGENERATOR_H_ */
