/*
 * CodeGenerator.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: flamedoge
 */

#include "CodeGenerator.h"
#include "Opt.h"
#include "Parser.h"
#include "Node.h"
#include <stack>
#include <string>
#include <fstream>

using namespace std;
namespace CodeGenWS
{
string prog;
void blockEval(Node *n)
{
  prog.append("{ ");
  defaultEval(n);
  prog.append(" }");
}
void parmEval(Node *n)
{
  prog.append("(");
  defaultEval(n);
  prog.append(")");
}
void declEval(Node *n)
{
  prog.append(n->children[0]->str).append(" ").append(n->children[1]->str);
}
void ifEval(Node *n)
{
  keywordEval("if", n);
  prog.append("(");
  evaluate(n->children[0]);
  prog.append(")");
  evaluate(n->children[1]);
}
void binopEval(Node *n)
{
  evaluate(n->children[0]);
  prog.append(n->str).append(" ");
  evaluate(n->children[1]);
}
void callEval(Node *n)
{
  prog.append(n->str).append("(");
  for(auto &c : n->children)
  {
    evaluate(c);
    prog.append(", ");
  }
  prog.erase(prog.end()-2, prog.end());
  prog.append(")");
}
void defaultEval(Node *n)
{
  cout << "  defaultEval: ";
  prog.append(n->str).append(" ");
  for(auto &c : n->children)
  {
    evaluate(c);
  }
  cout << "\n";
}
void keywordEval(const char *c, Node *n)
{
  prog.append(c).append(" ");
}
void evaluate(Node *n)
{
  cout << "evaluate: " << *n << endl;
  switch(n->id)
  {
  case DECL: declEval(n); break;
  case PARMS: parmEval(n); break;
  case BLOCK: blockEval(n); break;
  case IF: ifEval(n); break;
  case ELSE: keywordEval("else", n); break;
  case FOR: keywordEval("for", n); break;
  case WHILE: keywordEval("while", n); break;
  case RETURN: keywordEval("return", n); break;
  case BINOP: binopEval(n); break;
  case MINUS:
  {
    if (n->children.size() == 2)
      binopEval(n);
    else
      defaultEval(n);
    break;
  }
  case INT: prog.append(to_string(n->val)).append(" "); break;
  case STMT: defaultEval(n); prog.append("; "); break;
  case CALL: callEval(n); break;
  default: defaultEval(n); break;
  }
  //cout << "prog: " << prog << endl;
}

} // namespace CodeGenWS

int CodeGen::Process(Opt &opt, Parser &parser)
{
  auto top = parser.root;
  cout << endl << "======== CODEGEN ========" << endl;
  CodeGenWS::evaluate(top);

  cout << "C output: " << CodeGenWS::prog << endl;
  ofstream fileout;
  string outFilename("/tmp/");
  outFilename.append(to_string(rand() % 10000)).append(".c");
  fileout.open(outFilename);
  fileout << CodeGenWS::prog << endl;
  fileout.close();

  cout << "output written to " << outFilename << endl;
  return 0;
}


