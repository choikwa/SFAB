/*
 * Node.h
 *
 *  Created on: Mar 8, 2015
 *      Author: thinkdoge
 */

#ifndef NODE_H_
#define NODE_H_
#include <vector>

#include "Lexeme.h"
namespace AST
{
using namespace std;
using namespace Lexeme;
class Node
{
public:
  static inline unsigned nodeCount = 0;
  Node(){ cout << "Empty lexeme!" << endl; }
  Node(string str) : id(IDEN), str(str) {}
  Node(int id) : id(id), gid(nodeCount++) {}
  Node(int id, long in, int ln=0) : id(id), gid(nodeCount++), val(in), ln(ln) {}
  Node(int id, double in, int ln=0) : id(id), gid(nodeCount++), dval(in), ln(ln) {}
  Node(int id, char in, int ln=0) : id(id), gid(nodeCount++), str(1,in), ln(ln) {}
  Node(int id, const char *in, int ln=0) : id(id), gid(nodeCount++), str(in), ln(ln) {}
  Node(int id, const string &in, int ln=0) : id(id), gid(nodeCount++), str(in), ln(ln) {}
  bool operator==(const Node &n);
  bool operator!=(const Node &n) { return !(*this == n); }
  friend ostream& operator<<(ostream& os, const Node& dt);
  Node(int id, int numToAdd, Node *n, ...);
  Node(int id, const vector<Node*> &in) : id(id), gid(nodeCount++), children(in) {}

  void printTree();
  void printDot();

  string getval() const
  {
    switch (id)
    {
    case INT:
      return to_string(val);
    case FLOAT:
      return to_string(dval);
    default:
      return str;
    }
  }
  int id;  // lexeme enum
  int gid; // node id
  union
  {
    long val; // int
    double dval;  //floating point
  };
  string str;
  int ln;  // line number
  vector<Node*> children;
};

} /* namespace AST */

#endif /* NODE_H_ */
