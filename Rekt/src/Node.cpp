/*
 * Node.cpp
 *
 *  Created on: Mar 12, 2015
 *      Author: thinkdoge
 */


#include "Node.h"
#include <cstdarg>
#include <iostream>
#include <sstream>

namespace AST
{
using namespace std;
using namespace Lexeme;
bool Node::operator==(const Node &n)
{
  if (id != n.id) return false;
  if (id == INT && val != n.val) return false;
  if (id == FLOAT && dval != n.dval) return false;
  if (id == DECL)
  {
    return *this->children[0] == *n.children[0] &&
      *this->children[1] == *n.children[1];
  }
  if (id == PARMS)
  {
    for (unsigned i = 0; i < this->children.size(); i++)
    {
      if (!(*this->children[i] == *n.children[i]))
        return false;
    }
    return true;
  }
  if (getval() != n.getval()) return false;
  return true;
}

ostream& operator<<(ostream& os, const Node& n)
{
  
  if (n.getval() == "" || isKeyword(n.id))
    os << idToNameMap.at(n.id);
  else
    os << idToNameMap.at(n.id) << "'" << n.getval() << "'";
  return os;
}
Node::Node(int id, int numToAdd, Node *n, ...) : id(id)
{
  va_list vl;
  va_start(vl, n);
  children.push_back(n);
  for (int i = 0; i < numToAdd - 1; i++)
  {
    children.push_back(va_arg(vl, Node*));
  }
  va_end(vl);
  gid = Node::nodeCount++;
}
static int tab = 0;
void Node::printTree()
{
  for (int i = 0; i < tab; i++)
    cout << " ";
  cout << *this << endl;
  tab += 2;
  if (children.size() > 0)
  {
    for (auto c : children)
    {
      c->printTree();
    }
  }
  tab -= 2;
}

stringstream dotss;
void Node::printDot()
{
  if (children.size() > 0)
  {
    for (auto c : children)
      dotss << "\"" << *this << "_" << this->gid << "\" -> \"" << 
        *c << "_" << c->gid << "\"" << endl;

    for (auto c : children)
      c->printDot();
  }
}
} // namespace AST
