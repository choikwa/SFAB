/*
 * Lexer.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: thinkdoge
 */

#include "Lexer.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <cstring>
#include "Lexeme.h"
#include "Node.h"
#include "Opt.h"
#include "Rekt.h"

using namespace std;
using namespace Lexeme;
using namespace AST;

int Lexer::Process(const Opt &opt)
{
  cout << "======== LEXER ========" << endl;
  if (opt.src.empty())
  {
    cerr << "Source filename empty!" << endl;
    return FAIL::LEXER;
  }

  ifstream ifs(opt.src, ios::in);

  // read entire file into string
  std::string str;
  if (ifs)
  {
    // get length of file:
    ifs.seekg(0, ifs.end);
    int length = ifs.tellg();
    ifs.seekg(0, ifs.beg);

    str.resize(length, ' '); // reserve space
    char* begin = &*str.begin();

    ifs.read(begin, length);
    ifs.close();
  } else {
    cout << "Could not open source file" << endl;
    return FAIL::LEXER;
  }

  size_t it = 0;
  size_t ln = 1;
#define ADDLEXEME(x,y) cout << DN; lexemes.push_back(new Node(x, y, ln))
  while (str[it] != *str.end())
  {
    char c = str[it];
    switch(c)
    {
    case '#': processComments(str, it, ln); break;
    case '\n': ln++;  // discard whitespace, increment ln for \n
    case ' ':
    case '\t':
    case '\v':
    case '\f':
    case '\r': it++; cout << DN; break;
    case '\'':
    case '\"': processStrLit(str, it, ln); break;
    case ',': ADDLEXEME(COMMA, c); it++; break;
    case '.': ADDLEXEME(DOT, c); it++; break;
    case ';': ADDLEXEME(SEMICOLON, c); it++; break;
    case ':': ADDLEXEME(COLON, c); it++; break;
    case '-': ADDLEXEME(MINUS, c); it++; break;
    case '~':
    case '!': ADDLEXEME(UNOP, c); it++; break;
    default:
      if (string("@$[]{}()").find(c) != string::npos) { ADDLEXEME(BRACKET, c); it++; break; }
      // '-' is to be fixed up in parser as it can be unary as well
      if (string("+*/%").find(c) != string::npos) { ADDLEXEME(BINOP, c); it++; break; }
      if (string("<>=").find(c) != string::npos)
      {
        if (str[it+1] == '=') { ADDLEXEME(BINOP, string(1, c) + string("=")); it+=2; break; }
        else if (c == '=') { ADDLEXEME(ASSIGN, c); it++; break; }
        else { ADDLEXEME(BINOP, c); it++; break; }
      }
      if (string("&|").find(c) != string::npos)
      {
        if (str[it+1] == c) { ADDLEXEME(BINOP, str.substr(it, 2)); it+=2; break; }
        else { ADDLEXEME(BINOP, c); it++; break; }
      }
      if (isdigit(c)) { processNumLit(str, it, ln); break; }
      else if(isalpha(c)) { processIden(str, it, ln); break; }

      // unconsumed
      //cout << "Unrecognized Lexeme \'" << c << "\' in l:" << ln << endl;
      cout << c;
      it++;
      break;
    }
    cout.flush();
  }
#ifdef ADDLEXEME
#undef ADDLEXEME
#endif
  cout << endl;
  cout << "------------------------" << endl;
  for (auto it : lexemes)
  {
    cout << it->getval() << " ";
  }
  cout << endl;

  cout << "------------------------" << endl;
  for (auto it : lexemes)
  {
    cout << *it << " ";
  }
  cout << endl;
  return 0;
}

void Lexer::processComments(const string &str, size_t &it, size_t &ln)
{
  //multi-line comment
  if(0 && str[it+1] == '#')
  {
    //consume until '##'
    it += 2;
    while(!(str[it] == '#' && str[it+1] == '#'))
    {
      if(str[it] == '\n')
        ln++;
      if(str[it+1] == *str.end())
      {
        cout << "unterminated multi-line string in line " << ln << endl;
        return;
      }
      it++;
    }
    it++;
    return;
  }
  //single-line comment
  while(str[it+1] != '\n')
    it++;
  it++;
}
void Lexer::processStrLit(const string &str, size_t &it, const size_t ln)
{
  //read until unescaped ' or " is found.
  size_t start = it+1;
  char quote = str[it];
  while(true)
  {
    it++;
    if (str[it] == *str.end())
    {
      cout << "Unterminated string" << endl;
      return;
    }
    if (str[it] == quote &&
        str[it-1] != '\\')
      break;
  }
  lexemes.push_back(new Node(STR, str.substr(start, it - start), ln));
  it++;
  cout << DN;
}

void Lexer::processNumLit(const string &str, size_t &it, const size_t ln)
{
  size_t start = it;
  bool foundDecPoint = false;
  bool foundHex = false;
  while(isdigit(str[it]) || str[it] == '.' || str[it] == 'x')
  {
    if(str[it] == '.')
    {
      if(foundDecPoint)
        { cout << "l:" << ln << ": Found another decimal point while processing float literal" << endl; break; }
      else
        { foundDecPoint = true; }
    }
    else if(str[it] == 'x')
    {
      if(foundHex)
        { cout << "l:" << ln << ": Found another 'x' while processing hex literal" << endl; break; }
      else
        { foundHex = true; }
    }
    it++;
  }
  if(foundDecPoint)
    lexemes.push_back(new Node(FLOAT, stod(str.substr(start, it - start), NULL), ln));
  else
    lexemes.push_back(new Node(INT, stol(str.substr(start, it - start), NULL), ln));
}

void Lexer::processIden(const string &str, size_t &it, const size_t ln)
{
  size_t start = it;
  it++;
  while(isalpha(str[it]) || isdigit(str[it]) || str[it] == '_')
    it++;
  string word = str.substr(start, it - start);
  auto search = keywords.find(word);
  if (search != keywords.end())
    lexemes.push_back(new Node(search->second, search->first, ln));
  else
  {
    auto id = types.find(word);
    if (id != types.end())
      lexemes.push_back(new Node(TYPE, word, ln));
    else
      lexemes.push_back(new Node(IDEN, word, ln));
  }
}
