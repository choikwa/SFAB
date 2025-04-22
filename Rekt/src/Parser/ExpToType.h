#include "Parser.h"
#include <string>

#ifndef EXP_TO_TYPE_H_
#define EXP_TO_TYPE_H_
using namespace std;
namespace ParseWS
{
extern Parser *Parser;
Node *curFunc();
Node *TypeOf(Node *Exp)
{
  if (!Exp) return nullptr;
  if (Exp->id == TYPE) return Exp;
  string s = Exp->str;
  if (s == "ULIST") return new Node(TYPE, "[]");
  if (s == "UTUPLE") return new Node(TYPE, "@[]");
  if (s == "USET") return new Node(TYPE, "$[]");
  if (s == "UDICT") return new Node(TYPE, "{}");
  if (Exp->id == BINOP || Exp->id == UNOP) 
    return TypeOf(Exp->children[0]);
  if (Exp->id == IDEN)
  {
    cout << "TypeOf(" << Exp->str << ")";
    cout << *curFunc();
    // Look at SymTab
    Parser::SymTabEnt Sym(curFunc(), /*Type*/nullptr,
      Exp, 0); // operator== only checks FUNC, IDEN
    if (auto search = Parser->SymbolTable->find(Sym);
        search != Parser->SymbolTable->end()) {
      cout << " Found: " << *(*search).Type << endl;
      return (*search).Type;
    }
    cout << " didn't find." << endl;
    return nullptr;
  }
  if (Exp->id == CALL) return TypeOf(Exp->children[0]);
  if (Exp->id == INT) return new Node(TYPE, "int");
  if (Exp->id == FLOAT) return new Node(TYPE, "float");
  if (Exp->id == STR) return new Node(TYPE, "str");
  return nullptr;
}
} // namespace AST
#endif  // EXP_TP_TYPE_H_
