/*
 * Opt.cpp
 *
 *  Created on: Mar 2, 2015
 *      Author: thinkdoge
 */

#include "Opt.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <string>

#include "Rekt.h"

using namespace std;

Opt::Opt()
{
  opts =
  {
      {Preprocessor, {"Preprocessor", 0}},
      {Assembly,     {"Assembly", 0}},
      {Object,       {"Object", 0}},
      {Link,         {"Link", 0}},
      {Listing,      {"Listing", 0}},
  };
}

int Opt::Process(int argc, char **argv)
{
  cout << "======== Options ========" << endl;
  int c = 0;
  while ((c = getopt(argc, argv, "ESco:l")) != -1)
  {
    switch (c)
    {
    case 'E':
      opts[Preprocessor].on = 1;
      break;
    case 'S':
      opts[Assembly].on = 1;
      break;
    case 'c':
      opts[Object].on = 1;
      break;
    case 'o':
      opts[Link].on = 1;
      subopts[Link] = optarg;
      break;
    case 'l':
      opts[Listing].on = 1;
      break;
    default:
      break;
    }
  }
  bool useDefaultOpt = true;
  for(const auto &it : opts)
  {
  	if (it.second.on)
  	{
  		useDefaultOpt = false;
  		break;
  	}
  }

  if (useDefaultOpt)
  	opts[Object].on = 1;

  for(const auto &it : opts)
  {
    cout << it.second.optName << " " << it.second.on << " ";
    if (!subopts[it.first].empty())
      cout << subopts[it.first];
    cout << endl;
  }

  for(int idx = optind; idx < argc; idx++)
  {
    if (src.empty() && string(argv[idx]).rfind(".rek") != string::npos)
    {
      src = argv[idx];
      cout << "src: " << src << endl;
      continue;
    }
    cout << "Non-option arg: " << argv[idx] << endl;
    return FAIL::OPT+1;
  }
  if ((src.empty() && !opts[Link].on) ||
      (!opts[Preprocessor].on && !opts[Assembly].on && !opts[Object].on && !opts[Link].on))
  {
    cout << "Wrong arguments given" << endl;
    return FAIL::OPT+2;
  }

  return 0;
}
