/*
 * Opt.h
 *
 *  Created on: Mar 2, 2015
 *      Author: thinkdoge
 */

#ifndef OPT_H_
#define OPT_H_
#include <unordered_map>
#include <string>
#include <vector>
using namespace std;
enum Options
{
  Preprocessor,
  Assembly,
  Object,
  Link,
  Listing,
};

struct state
{
  const char *optName;
  bool on;
};

class Opt
{
public:
  Opt();
  int Process(int argc, char **argv);
  unordered_map<int, state> opts;
  unordered_map<int, string> subopts;
  string src;
  vector<string> objs;
};

#endif /* OPT_H_ */
