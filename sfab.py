#!/usr/bin/env python3

import glob
import ast
import os
import sys

clean = False
verbose = True
dryrun = False

if len(sys.argv) > 1:
  if sys.argv[1] == "clean":
    clean = True
  elif sys.argv[1] == "verbose":
    verbose = True
  elif sys.argv[1] == "dry":
    dryrun = True

cmds = []
compiler = "clang++"
compiler_flags = "-O2 -g -std=c++20 -w"
libs = []
gobjs = []      # obj list for bookkeeping
binary = ""

def readSFAB():
  ss = ""
  if not os.path.exists("_SFAB"):
    print("_SFAB doesn't exist")
    exit()
  with open("_SFAB") as fin:
    ss = fin.read()
  return ss

# top level _SFAB
ss = readSFAB()
mydeptree = ast.literal_eval(ss)

# expand wildcards
def expand_wildcards(deptree):
  for k in list(deptree):
    v = deptree[k]
    
    # key = ""
    if k[:2] == "*.":
      wildcard_files = glob.glob(k)  # ex. *.cpp
      for wf in wildcard_files:
        deptree[wf] = v
      deptree.pop(k) # remove wildcard entry from deptree

    # value = ["","",..]
    for e in v:
      if e[:2] == "*.":
        wildcard_files = glob.glob(e)
        for wf in wildcard_files:
          deptree[k].append(wf)
        deptree[k].remove(e) # remove wildcard entry from v

  #print("wildcard expanded deptree:")
  #print(deptree)

expand_wildcards(mydeptree)

dir_stack = [os.getcwd()]
def pushd(path):
  os.chdir(path)
  dir_stack.append(path)
  #print("pushd: " + path)

def popd():
  if len(dir_stack) > 0:
    dir_stack.pop()
    path = "/".join(dir_stack)
    os.chdir(path)
    #print("popd: " + path)
  else:
    print("Empty dir_stack!")

def getcurpath():
  return os.path.abspath("/".join(dir_stack))


def keyword(k):
  return k == "_LIBRARY" or k == "_BINARY" or k == "_PATH"

# iterate directories?
forest = [[getcurpath(), mydeptree]]
# Recursively iterate subdirs
# basic, include all seen directories for compilation
include_dirs = "-I" + getcurpath()
seenpath = [getcurpath()]
# pathtree = [path, deptree]
def recurse_dir(pathtree):
  global include_dirs
  tree = pathtree[1]
  for k,v in tree.items():
    if not keyword(k):
      for e in v:
        if os.path.isdir(e):
          newdir = os.path.abspath(getcurpath() + "/" + e)
          if newdir in seenpath:
            # move tree to front in forest
            seenidx = -1
            for idx, tree in enumerate(forest):
              if newdir == tree[0]:
                seenidx = idx
            if seenidx != -1:
              forest.insert(0, forest.pop(seenidx))
          else:
            pushd(e)
            seenpath.append(newdir)
            ss = readSFAB()
            dt = ast.literal_eval(ss)
            expand_wildcards(dt)
            new_pathtree = [getcurpath(), dt]
            include_dirs += " -I" + getcurpath()
            forest.insert(0, new_pathtree)
            recurse_dir(new_pathtree)
            popd()
recurse_dir(forest[0])

if verbose:
  print("forest:")
  for tree in forest:
    print(str(tree))

# Generate dgraph from forest of deptrees
# we do this in several steps.
# - enumerate list of nodes from forest
# - create digraph from node indices
# - associate each node to cmd later
def static_lib_name(name):
  return "lib" + name + ".a"
def has_filetype(name):
  return name.find('.') != -1

# nodes = [[path, filename], ..]
nodes = []
# dgraph = [[1,2], [3,4], ..] means 0th node has incoming edge from [1st, 2nd] node, etc.
dgraph = [[]]
def gen_dgraph(forest):
  global nodes

  # k = filename
  def find_idx(nodes, path, k):
    for i,n in enumerate(nodes):
      if n[0] == path and n[1] == k:
        return i
    print("Could not find idx for: " + k)

  # enumerate list of nodes
  for pt in forest:
    path, tree = pt
    for k,v in tree.items():
      for e in v:
        if os.path.isdir(path + "/" + e):
          continue
        if k == "_LIBRARY":
          e = static_lib_name(e)
        nodes.append([path, e])
  print("enumerated node list:")
  for i,n in enumerate(nodes):
    print(str(i) + ": " + str(n))

  for i in range(len(nodes)):
    dgraph.append([])
  for pt in forest:
    path, tree = pt
    is_library = True
    for k,v in tree.items():
      for e in v:
        if k == "_BINARY" or has_filetype(k):
          is_library = False
        if k == "_LIBRARY" or k == "_BINARY":
          continue

        pidx = -1  # p -> q
        qidx = -1
        #print("Finding idx for (path, k, e): " + path + ", " + k + ", " + e)
        if os.path.isdir(path + "/" + e):
          newe = e[e.rfind("/")+1:]
          pidx = find_idx(nodes, os.path.abspath(path + "/" + e), static_lib_name(newe))
        else:
          pidx = find_idx(nodes, path, e)

        newk = k
        if is_library:
          newk = static_lib_name(k)
        qidx = find_idx(nodes, path, newk)

        #print("Inserting edge : " + str(pidx) + "->" + str(qidx))
        dgraph[qidx].append(pidx)
  return dgraph
gen_dgraph(forest)

for i,d in enumerate(dgraph):
  print(str(i) + ": " + str(d))

# Generate commands
# path = str
# k = key
# tree = dict
def recurse_tree(path, k, tree):
  global cmds
  global libs
  v = tree[k]
  for e in v:
    if e in tree:
      recurse_tree(path, e, tree)
    if e[-4:] == ".cpp":
      cmd = compiler + " " + compiler_flags + " " + include_dirs + " -c -o " \
        + e[:-4] + ".o " + e
      objs.append(path + "/" + e[:-4] + ".o")
      gobjs.append(path + "/" + e[:-4] + ".o")
      cmds.append([path, cmd])
    elif e[-3:] == ".py":
      cmd = "python3 " + e
      cmds.append([path, cmd])
      gobjs.append(path + "/" + k)  # bookkeep generated .txt for removal

for pt in forest:
  p,t = pt
  k = ""
  objs = []
  if "_BINARY" in t:
    k = "_BINARY"
  elif "_LIBRARY" in t:
    k = "_LIBRARY"
  else:
    print("_BINARY or _LIBRARY key not found in tree: " + str(t))
    exit()
  recurse_tree(p, k, t)
  if k == "_BINARY":
    binary = t[k][0]
    cmd = compiler + " " + compiler_flags + " " + " ".join(objs) + " " + " ".join(libs) \
      + " -o " + binary
    cmds.append([p, cmd])
  elif k == "_LIBRARY":
    lib = "lib" + t[k][0] + ".a"
    cmd = "ar qc " + lib + " " + " ".join(objs)
    cmds.append([p, cmd])
    cmd = "ranlib " + lib
    cmds.append([p, cmd])
    libs.append(p + "/" + lib)
    

#print("----------cmds----------")
if clean:
  print("rm " + binary)
  os.system("rm " + binary)
  for lib in libs:
    print("rm " + lib)
    os.system("rm " + lib)
  for obj in gobjs:
    print("rm " + obj)
    os.system("rm " + obj)
else:
  for pathcmd in cmds:
    path, cmd = pathcmd
    print(cmd)
    if not dryrun:
      pushd(path)
      rc = os.system(cmd)
      popd()
      if rc != 0:
        print("Failed to compile cmd: " + cmd)
        exit()

# todo: parallel processing
# - construct dependence graph
# - topological sort from dependence graph
# - determine parallel regions by forming groups that have no incoming edges, 
#     then removing those nodes to reveal next set
# - critical path
# - apply resource constraints

