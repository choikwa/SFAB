import glob
import ast
import os
import sys

clean = False
if len(sys.argv) > 1:
  if sys.argv[1] == "clean":
    clean = True

def readSFAB():
  ss = ""
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
  return "/".join(dir_stack)


def keyword(k):
  return k == "_LIBRARY" or k == "_BINARY" or k == "_PATH"

# iterate directories?
forest = [[getcurpath(), mydeptree]]
#print("Recursively iterate subdirs")

# basic, include all seen directories for compilation
include_dirs = "-I" + getcurpath()

# pathtree = [path, deptree]
def recurse_dir(pathtree):
  global include_dirs
  tree = pathtree[1]
  for k,v in tree.items():
    if not keyword(k):
      for e in v:
        if os.path.isdir(e):
          pushd(e)
          ss = readSFAB()
          dt = ast.literal_eval(ss)
          expand_wildcards(dt)
          new_pathtree = [getcurpath(), dt]
          include_dirs += " -I" + getcurpath()
          forest.insert(0, new_pathtree)
          recurse_dir(new_pathtree)
          popd()
recurse_dir(forest[0])

print("forest:")
for tree in forest:
  print(str(tree))

# Generate commands
cmds = []
compiler = "g++"
compiler_flags = "-O2 -g -std=c++17"
libs = []
gobjs = []
binary = ""

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
    pushd(path)
    rc = os.system(cmd)
    popd()
    if rc != 0:
      print("Failed to compile cmd: " + cmd)
      exit()

