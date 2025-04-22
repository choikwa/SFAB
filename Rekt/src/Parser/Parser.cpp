/*
 * Parser.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: thinkdoge
 */

#include "Parser.h"
#include "Lexeme.h"
#include "Lexer.h"
#include "Node.h"
#include "Rekt.h"
#include "ExpToType.h"
#include <queue>
#include <deque>
#include <vector>
#include <cstring>
#include <memory>
#include <stack>
#include <cassert>
#include <fstream>
#include <sstream>

#define assertm(exp, msg) assert((void(msg), exp));
using namespace Lexeme;
using namespace std;
Parser::Parser() : root(NULL) 
{
  SymbolTable = make_unique<unordered_set<SymTabEnt>>();
  FuncTable = make_unique<unordered_set<FuncEnt>>();
}

Parser::~Parser()
{
  // TODO Auto-generated destructor stub
}

using namespace std;

using namespace AST;
static vector<Node*> *ss = NULL;
namespace ParseWS
{
class Parser *Parser = nullptr;
static size_t idx = 0;
static int tab = -2;
static bool trace = getenv("traceParser");
stack<Node *> FuncStack;

#define ENT() auto __old = idx; \
  { if(idx >= ss->size()) { cout << "end of lex stream" << endl; return NULL; } \
    if(trace) { tab+=2; for(int d=tab; d>0; d--){ cout << " "; } \
    cout << __func__ << ": " << *ss->at(idx) << endl; }}
#define RET(x) do{ if(x==NULL) idx = __old; tab-=2; return x; } while(false);
#define ERR(n) do{ cout << __FILE__ << ":" << __LINE__ << ": !!! Error: Expected " << Node(n) << " at ln" << \
		curNode().ln << endl; exit(0); } while(false);
void error(Node n1, Node n2) { cout << "!!! Error: Expected " << n1 << " or " << n2 <<
  " at ln" << curNode().ln << endl; exit(0); }
void error(Node n1, Node n2, Node n3) { cout << "!!! Error: Expected " << n1 << " or " << n2 <<
  " or " << n3 << " at ln" << curNode().ln << endl; exit(0); }

Node *expect(Node n)	// throw error if not matched
{
  Node *ret;
  if((ret = match(n)))
    return ret;
  else
    ERR(n);
  return NULL;
}
Node *expect(Node n1, Node n2)
{
  Node *ret;
  if((ret = match(n1)) || (ret = match(n2)))
    return ret;
  else
    error(n1, n2);
  return NULL;
}
Node *lexExpect(int id)
{
  Node *ret;
  if((ret = lexMatch(id)))
    return ret;
  else
    cout << "!!! Expected " << idToNameMap.at(id) << " at ln" << curNode().ln << endl;
  return NULL;
}
void printTape()
{
	cout << "Tape: ";
	for(auto i = idx; i < idx + 5 && i < ss->size(); i++)
	{
		cout << *ss->at(i) << " ";
	}
	cout << endl;
}

Node *prevNode() { return idx > 0 ? ss->at(idx-1) : Parser->root; }

Node &curNode() 
{
  assertm(idx < ss->size(), "idx > ss->size()");
  return *ss->at(idx);
}

Node *curFunc() { return FuncStack.top(); }

Node *getNode()
{
  if (idx >= ss->size())
  {
    //cout << "end of lexeme stream" << endl;
    return NULL;
  }
//  cout << "ss[" << idx << "]=" << *ss->at(idx) << endl;
  Node *tmp = ss->at(idx);
  return tmp;
}
Node *lexMatch(int id)	// Only match the id and advance
{
  ENT();
  Node *tmp = getNode();
  if (tmp && tmp->id == id)
  {
    idx++;
    RET(tmp);
  }
  RET(NULL);
}

Node *match(Node n)	// Full match unless keyword and advance
{
  ENT();
  if(isKeyword(n.id))
    return lexMatch(n.id);

  Node *tmp = getNode();
  if (tmp && *tmp == n)
  {
    idx++;
    RET(tmp);
  }
  RET(NULL);
}

void TypeCheck(Node *p, Node *q)
{
  if (*TypeOf(p) != *TypeOf(q))
  {
    cout << "Type mismatch detected: ";
    idx--;
    ERR(*TypeOf(p));
  }
}

/////////////////
Node *program()
{
  ENT();
  FuncStack.push(new Node(FUNC, "GLOBAL"));
  vector<Node*> stmts;
  while(Node *type = stmt()) { stmts.push_back(type); }
  RET(new Node(PROGRAM, stmts));
}
Node *stmt()
{
  ENT();
  Node *n, *e, *assign;
  if((n = func()) ||
     (n = f_if()) ||
     (n = f_for()) ||
     (n = f_while()) ||
     (n = f_switch()))
  {
    RET(new Node(STMT, 1, n));
  }
  else if((n = decl()))
  {
    if(lexMatch(SEMICOLON))
      RET(new Node(STMT, 1, n));
    if((assign = lexExpect(ASSIGN)))
    {
      if((e = exp()))
      {
        if(lexExpect(SEMICOLON))
        {
          // Type Check
          TypeCheck(n->children[0], e);
          RET(new Node(STMT, 3, n, assign, e));
        }
      } else ERR(EXP);
    }
  }
  else if ((n = lexMatch(IDEN)))
  {
    if((assign = lexExpect(ASSIGN)))
    {
      if((e = exp()))
      {
        // Type Check
        // todo: is this necessary if IDEN gets new def?
        Parser::SymTabEnt Sym(curFunc(), /*Type*/nullptr,
          n, 0); // operator== only checks FUNC, IDEN
        if (auto search = Parser->SymbolTable->find(Sym);
            search != Parser->SymbolTable->end())
        {
          auto Type = (*search).Type;
          TypeCheck(Type, e);
        }
        if(lexExpect(SEMICOLON))
        {
          RET(new Node(STMT, 3, n, assign, e));
        }
      } else ERR(EXP);
    }
  }
  else if ((n = lexMatch(RETURN)))
  {
    if((e = exp()))
    {
      if(lexExpect(SEMICOLON))
      {
        RET(new Node(STMT, 2, n, e));
      }
    } else ERR(EXP);
  }
  else if ((n = call()))
  {
    if(lexMatch(SEMICOLON))
    {
      RET(new Node(STMT, 1, n));
    } else ERR(EXP);
  }
  RET(NULL);
}
string parmsToString(Node *parms) {
  if (!parms)
    return "";

  string ss("(");
  if (parms->children.size() > 0)
  {
    Node *e = parms->children[0]; // Decl's
    ss.append(e->children[0]->str); // Type
    ss.append(" ");
    ss.append(e->children[1]->str); // Iden
    for (unsigned i = 1; i < parms->children.size(); i++) {
      e = parms->children[i];
      ss.append(", ");
      ss.append(e->children[0]->str);
      ss.append(" ");
      ss.append(e->children[1]->str);
    }
  }
  ss.append(")");
  return ss;
}
Node *func()
{
  ENT();
  if(Node *n_decl = decl())
  {
    auto *IncompleteFunc = new Node(FUNC, 1, n_decl);
    IncompleteFunc->str = n_decl->children[1]->str; //IDEN
    FuncStack.push(IncompleteFunc);
    if (Node *n_args = parms())
    {
      // Add to FuncTable, check ODR
      Parser::FuncEnt FnEnt(n_decl, n_args, prevNode()->ln);
      if (auto search = Parser->FuncTable->find(FnEnt);
          search != Parser->FuncTable->end())
      {
        cout << "!!! Error: multiple definition violates ODR {"
          << *FnEnt.Decl->children[0] << ' '
          << *FnEnt.Decl->children[1] << ' '
          << parmsToString(FnEnt.Parms) << "} at ln " <<
          FnEnt.lineno << endl;
        cout << "  Previously defined at ln" <<
          (*search).lineno << endl;
        exit(0);
      }
      else 
      {
        cout << "FuncTable: inserting "
          << *FnEnt.Decl->children[1] 
          << endl;
        Parser->FuncTable->insert(FnEnt);
      }
      if (Node *n_block = block())
      {
        Node *fn = new Node(FUNC, 3, n_decl, n_args, n_block);
        FuncStack.pop();
        RET(fn);
      } else ERR(BLOCK);
    }
    FuncStack.pop();
  }
  RET(NULL);
}
Node *parms()
{
  ENT();
  vector<Node*> children;
  if (match(Node(BRACKET, "(")))
  {
    if (Node *decl1 = decl())
    {
      children.push_back(decl1);
      Node *comma = nullptr;
      Node *decl2 = nullptr;
      do
      {
        comma = lexMatch(COMMA);
        decl2 = decl();
        if (comma && decl2)
          children.push_back(decl2);
      } while (comma && decl2);
    }
    if (match(Node(BRACKET, ")")))
    {
      RET(new Node(PARMS, children));
    } else 
      expect(Node(BRACKET, ")"));
  }
  RET(NULL);
}
Node *decl()
{
  ENT();
  if (Node *n_type = type())
  {
    if (Node *iden = lexExpect(IDEN))
    {
      // Add to SymTab, check one definition rule (ODR)
      Parser::SymTabEnt Sym(curFunc(), n_type, 
        iden, prevNode()->ln);
      if (auto search = Parser->SymbolTable->find(Sym);
          search != Parser->SymbolTable->end())
      {
        cout << "!!! Error: multiple definition violates ODR {" 
          << *Sym.Func << " " << *Sym.Type << " " << *Sym.Iden << 
          "} at ln " << Sym.lineno << endl;
        cout << "  Previously defined at ln" << 
          (*search).lineno << endl;
        exit(0);
      }
      else
      {
        cout << "SymTable: inserting " << *curFunc() << " " << 
          *Sym.Type << " " << *Sym.Iden << endl;
        Parser->SymbolTable->insert(Sym);
      }
      RET(new Node(DECL, 2, n_type, iden));
    }
  }
  RET(NULL);
}
Node *type()
{
  ENT();
  if(Node *t = lexMatch(TYPE)) { RET(t); } // int, float, char
  if(match(Node(BRACKET, "[")) && match(Node(BRACKET, "]"))) RET(new Node(TYPE, "[]"));
  if(match(Node(BRACKET, "@")) && 
     match(Node(BRACKET, "[")) && match(Node(BRACKET, "]"))) RET(new Node(TYPE, "@[]"));
  if(match(Node(BRACKET, "$")) && 
     match(Node(BRACKET, "[")) && match(Node(BRACKET, "]"))) RET(new Node(TYPE, "$[]"));
  if(match(Node(BRACKET, "{")) && match(Node(BRACKET, "}"))) RET(new Node(TYPE, "{}"));
  RET(NULL);
}
Node *block()
{
  ENT();
  if(match(Node(BRACKET, "{")))
  {
    vector<Node *> stmts;
    while(Node *s = stmt())
    {
      stmts.push_back(s);
    }
    if(match(Node(BRACKET, "}")))
      RET(new Node(BLOCK, stmts));
  }
  RET(NULL);
}
Node *f_if()
{
  ENT();
  if(match(IF))
  {
    vector<Node*> chl;
    if(expect(Node(BRACKET, "(")))
    {
      if(Node *e = exp())
      {
        chl.push_back(e);
        if(expect(Node(BRACKET, ")")))
        {
          if(Node *s = stmt())
          {
            chl.push_back(s);
          }
          else if (Node *b = block())
          {
            chl.push_back(b);
          }
          else
            error(STMT, BLOCK);

          if(Node *n_else = f_else())
          {
            chl.push_back(n_else);
          }
          RET(new Node(IF, chl));
        }
      }
    }
  }
  RET(NULL);
}
Node *f_else()
{
  ENT();
  vector<Node *> chl;
  if(match(ELSE))
  {
    if(Node *n_if = f_if()) // else if case
    {
      chl.push_back(n_if);
      RET(new Node(ELSE, chl));
    }
    else
    {
      if(Node *s = stmt())
      {
        chl.push_back(s);
      }
      else if (Node *b = block())
      {
        chl.push_back(b);
      }
      else
      {
        error(STMT, BLOCK);
        RET(NULL);
      }
      RET(new Node(ELSE, chl));
    }
  }
  RET(NULL);
}

Node *dictent()
{
  ENT()
  if (Node *key = exp())
  {
    if (lexExpect(COLON))
    {
      if (Node *value = exp())
      {
        Node *n = new Node(EXP, 2, key, value);
        n->str = "DICTENT";
        RET(n);
      } else ERR("Value exp of dict key-value pair");
    }
  }
  RET(NULL);
}

Node *exp()
{
  ENT();
  Node *n = NULL, *op = NULL, *e = NULL;
  if (match(Node(BRACKET, "(")))
  {
    // parenthesis exp
    if ((e = exp()))
    {
      expect(Node(BRACKET, ")"));
    }
    else
      error(EXP, BINOP, UNOP);

    n = new Node(BRACKET, 1, e);
  }
  else if (match(Node(BRACKET, "[")))
  {
    // unnamed list exp = null or comma separated exp's
    vector<Node*> children;
    if ((e = exp()))
    {
      children.push_back(e);
      Node *comma = nullptr;
      Node *e2 = nullptr;
      do
      {
        comma = lexMatch(COMMA);
        e2 = exp();
        if (comma && e2)
          children.push_back(e2);
      } while (comma && e2);
    }
    if (match(Node(BRACKET, "]")))
    {
      n = new Node(EXP, children);
      n->str = "ULIST";
    } else
      expect(Node(BRACKET, "]"));
  }
  else if (match(Node(BRACKET, "@")) && match(Node(BRACKET, "[")))
  {
    // unnamed tuple exp = null or comma separated exp's
    vector<Node*> children;
    if ((e = exp()))
    {
      children.push_back(e);
      Node *comma = nullptr;
      Node *e2 = nullptr;
      do
      {
        comma = lexMatch(COMMA);
        e2 = exp();
        if (comma && e2)
          children.push_back(e2);
      } while (comma && e2);
    }
    if (match(Node(BRACKET, "]")))
    {
      n = new Node(EXP, children);
      n->str = "UTUPLE";
    } else
      expect(Node(BRACKET, "]"));
  }
  else if (match(Node(BRACKET, "$")) && match(Node(BRACKET, "[")))
  {
    // unnamed set exp = null or comma separated exp's
    vector<Node*> children;
    if ((e = exp()))
    {
      children.push_back(e);
      Node *comma = nullptr;
      Node *e2 = nullptr;
      do
      {
        comma = lexMatch(COMMA);
        e2 = exp();
        if (comma && e2)
          children.push_back(e2);
      } while (comma && e2);
    }
    if (match(Node(BRACKET, "]")))
    {
      n = new Node(EXP, children);
      n->str = "USET";
    } else
      expect(Node(BRACKET, "]"));
  }
  else if (match(Node(BRACKET, "{")))
  {
    // unnamed dict exp = null or comma separated (exp ':' exp)
    vector<Node*> children;
    if ((e = dictent()))
    {
      children.push_back(e);
      Node *comma = nullptr;
      Node *e2 = nullptr;
      do
      {
        comma = lexMatch(COMMA);
        e2 = dictent();
        if (comma && e2)
          children.push_back(e2);
      } while (comma && e2);
    } 
    if (match(Node(BRACKET, "}")))
    {
      n = new Node(EXP, children);
      n->str = "UDICT";
    } else
      expect(Node(BRACKET, "}"));
  }
  else if ((e = call()) || (e = lexMatch(IDEN)) || (e = lexMatch(INT)) || 
           (e = lexMatch(FLOAT)) || (e = lexMatch(STR)))
  {
    n = e;
  }
  else if ((op = lexMatch(UNOP)) || (op = lexMatch(MINUS)))
  {
    if ((e = exp()))
    {
      n = new Node(op->id, 1, e);
      n->str = op->str;
    }
    else
      ERR(EXP);
  }

  if (n)
  {
    Node *tmp = r_exp(n);
    if (tmp)
      n = tmp;
    
    // Fix up operator precedence if there are any seq of binops.
    n = fixUpOpPrec(n);
    RET(n);
  }
  RET(NULL);
}

Node *r_exp(Node *n)
{
  ENT();
  Node *op = NULL, *e = NULL;
  while (true)
  {
    bool found = false;
    if (n->id == CALL || n->id == IDEN)
    {
      if ((op = lexMatch(DOT)))
      {
        if ((e = exp()))
        {
          found = true;
          n = new Node(EXP, 2, n, e);
          n->str = op->str;
        }
        else
          ERR(EXP);
      }
      else if (match(Node(BRACKET, "[")))
      {
        idx--;
        while (match(Node(BRACKET, "[")))
        {
          if ((e = exp()))
          {
            // todo: match type of call/iden to exp inside '[]'
            if (expect(Node(BRACKET, "]")))
            {
              found = true;
              n = new Node(EXP, 2, n, e);
              n->str = "[]";
            }
          }
          else
            ERR(EXP);
        }
      }
    }

    if((op = lexMatch(BINOP)) || (op = lexMatch(MINUS)) || (op = lexMatch(ASSIGN)))
    {
      if((e = exp()))
      {
        TypeCheck(n, e);
        found = true;
        n = new Node(op->id, 2, n, e);
        n->str = op->str;
      }
      else
        ERR(EXP);
    }

    if (!found)
      break;
  }
  if (n)
    RET(n);
  RET(NULL);
}

Node *fixUpOpPrec(Node *n)
{
  auto top = n;

  //UNOPs
  /*
   * MINUS
   *   UNOP
   *     { (), [], . }   // stop if these seen.
   */
  if(top->id == UNOP || top->id == MINUS)
  {
    auto tmp = top;
    auto prevTmp = tmp;
    while(tmp->id == UNOP || tmp->id == MINUS)
    {
      prevTmp = tmp;
      tmp = tmp->children[0];
    }
    auto lastUNOP = prevTmp; //cout << "lastUNOP" << endl; lastUNOP->printTree();
    auto firstNonUNOP = tmp; //cout << "firstNonUNOP" << endl; firstNonUNOP->printTree();

    while(!(tmp->id == UNOP ||
            (tmp->id == MINUS && tmp->children.size() == 1) ||   // can't skip UNOP MINUS
            tmp->id == BRACKET ||
            tmp->id == DOT) &&
          tmp->children.size() > 0)
    {
      prevTmp = tmp;
      tmp = tmp->children[0];
    }
    auto newParent = prevTmp; //cout << "newParent" << endl; newParent->printTree();
    auto stop = tmp;   //cout << "stop" << endl; stop->printTree();

    if (lastUNOP != newParent)
    {
      newParent->children[0] = top;
      lastUNOP->children[0] = stop;
      top = firstNonUNOP;
    }
    //cout << "final result" << endl;
    //top->printTree();
  }

  //BINOPs
  std::queue<Node*> Q;
  Q.push(top);
  while(!Q.empty())
  {
    auto node = Q.front(); Q.pop();
    if(node->id == BINOP || (node->id == MINUS && node->children.size() > 1))
    {
      auto secondChild = node->children[1];

      /*
       * BINOP'*'       BINOP'+'
       *   INT'1'         BINOP'*'
       *   BINOP'+' -->     INT'1'
       *     INT'2'         INT'2'
       *     INT'3'       INT'3'
       */
      if(secondChild->id == BINOP && hasHigherOpPrecedence(node->str, secondChild->str))
      {
        top = secondChild;
        auto tmp2 = top->children.front();
        n->children[1] = tmp2;
        top->children[0] = n;
        Q.push(top);
      }
    }
  }
  return top;
}

enum
{
  OPMULDIVMOD=0,
  OPADDSUB,
  OPCMP,
  OPBITWISEAND,
  OPBITWISEOR,
  OPXOR,
  OPAND,
  OPOR,
  OPASSIGN,
  OPINVALID,
};
static int enumifyOp(const string &s1)
{
  auto cstr = s1.c_str();
  if(strpbrk(cstr, "*/%"))
    return OPMULDIVMOD;
  if(strpbrk(cstr, "+-"))
    return OPADDSUB;
  if(strpbrk(cstr, "<>!") || strstr(cstr, "=="))
    return OPCMP;
  if(strstr(cstr, "&&"))
    return OPAND;
  if(strstr(cstr, "||"))
    return OPOR;
  if(strchr(cstr, '^'))
    return OPXOR;
  if(strchr(cstr, '&'))
    return OPBITWISEAND;
  if(strchr(cstr, '|'))
    return OPBITWISEOR;
  return OPINVALID;
}
bool hasHigherOpPrecedence(const string &s1, const string &s2)
{
if (enumifyOp(s1) < enumifyOp(s2))
  return true;
return false;
}

Node *f_for()
{
  ENT();
  if(lexMatch(FOR))
  {
    if(Node *iter = iterator())
    {
      if(Node *b = block())
      {
        RET(new Node(FOR, 2, iter, b));
      } else ERR(BLOCK);
    } else ERR(ITERATOR);
  }
  RET(NULL);
}
Node *iterator()
{
  ENT();
  if(match(Node(BRACKET, "(")))
  {
    vector<Node*> chl;
    if(Node *d = decl())
    {
      chl.push_back(d);
      Node *comma;
      do
      {
        comma = lexMatch(COMMA);
        d = decl();
        if (comma && d)
        {
          chl.push_back(d);
        }
      } while(comma && d);
      if(expect(Node(BINOP, "|")))
      {
        Node *cond;
        if((cond = exp()))
        {
          chl.push_back(cond);
          do
          {
            comma = lexMatch(COMMA);
            cond = exp();
            if (comma && cond)
            {
              chl.push_back(cond);
            }
          } while(comma && cond);
        }
        else if (Node *it = iterable())
        {
          chl.push_back(it);
        }
        else 
          error(EXP, Node("ITERABLE"));  // Need EXP(s) or ITERABLE
        
        if(expect(Node(BRACKET, ")")))
        {
          RET(new Node(ITERATOR, chl));
        }
      }
    } else ERR(DECL);
  }
  RET(NULL);
}

Node *iterable()
{
  ENT();
  if (Node *n = lexMatch(IDEN))
  {
    //todo: consult SymbolTable to check iterable
    
  }
  RET(NULL);
}

Node *call()
{
  ENT();
  auto oldIdx = idx;
  if(Node *n = lexMatch(IDEN))
  {
    vector<Node*> chl;
    if(match(Node(BRACKET, "(")))
    {
      if(Node *e = exp())
      {
        chl.push_back(e);
        Node *comma;
        do
        {
          if((comma = lexMatch(COMMA)) && (e = exp()))
            chl.push_back(e);
        } while(comma && e);
      }
      if(expect(Node(BRACKET, ")")))
      {
        Node *call = new Node(CALL, chl);
        call->str = n->str;
        RET(call);
      }
    }
  }
  idx = oldIdx;
  RET(NULL);
}
Node *f_while()
{
  ENT();
  if(lexMatch(WHILE))
  {
    if(expect(Node(BRACKET, "(")))
    {
      if(Node *e = exp())
      {
        if(expect(Node(BRACKET, ")")))
        {
          if(Node *b = block())
          {
            RET(new Node(WHILE, 2, e, b));
          } else ERR(BLOCK);
        }
      } else ERR(EXP);
    }
  }
  RET(NULL);
}
Node *f_switch()
{
  ENT();
  if(lexMatch(SWITCH))
  {
    if(expect(Node(BRACKET, "(")))
    {
      if(Node *e = exp())
      {
        vector<Node *> chl;
        chl.push_back(e);
        if(expect(Node(BRACKET, ")")) &&
           expect(Node(BRACKET, "{")))
        {
          Node *test;
          while((test = exp()) ||
                (test = lexMatch(ALL)) ||
                (test = lexMatch(NONE)))
          {
            if(lexExpect(COLON))
            {
              vector<Node*> tmp;
              while(Node *s = stmt())
              {
                tmp.push_back(s);
              }
              Node *mycase = new Node(CASE, 1, test);
              mycase->children.insert(mycase->children.end(), tmp.begin(), tmp.end());
              chl.push_back(mycase);
            }
          }
          if(chl.size() < 2) error(EXP, ALL, NONE); // need at least one case
          if(expect(Node(BRACKET, "}")))
            RET(new Node(SWITCH, chl));
        }
      }
    }
  }
  RET(NULL);
}

}
namespace AST
{
extern stringstream dotss;
}
int Parser::Process(Lexer &lex)
{
  cout << endl << "======== PARSER ========" << endl;
  ss = &lex.lexemes;
  ParseWS::Parser = this;
  root = ParseWS::program();  // recursive descent parsing!

  cout << "-------- AST TREE --------" << endl;
  if (root)
  {
    root->printTree();
    bool generateDotGraph = true;
    if (generateDotGraph) {
      root->printDot();
      string dotout("Digraph G {\n");
      dotout.append(AST::dotss.str()).append("\n}\n");
      string outFN("/tmp/");
      outFN.append(to_string(rand() % 1000)).append(".dot");
      ofstream fout;
      fout.open(outFN);
      fout << dotout << endl;
      fout.close();
      cout << "dot output writen to " << outFN << endl;
    }
  }
  if (ParseWS::idx != ss->size())
  {
    Node *fail = ss->at(ParseWS::idx);
    cout << "Failed at parsing " << *fail << " at ln" << fail->ln << endl;
    return FAIL::PARSER;
  }
  bool dumpFuncs = true;
  if (dumpFuncs)
  {
    cout << "-------- Funcs --------" << endl;
    for (const auto &e : *FuncTable)
    {
      cout << *e.Decl->children[0] << ' ' 
        << *e.Decl->children[1] << ' ' 
        << ParseWS::parmsToString(e.Parms) << ' '
        << " ln" << e.lineno << '\n';
    }
  }

  bool dumpSymbols = true;
  if (dumpSymbols) 
  {
    cout << "-------- Symbols --------" << endl;
    for (const auto &e : *SymbolTable)
    {
      cout << *e.Func << ' ' << *e.Type << ' ' << *e.Iden << 
        " ln" << e.lineno << '\n';
    }
  }
  return 0;
}
