
filein = open("Lexemes.txt", 'r')
idToNameFO = open("idToNameMap.txt", 'w')

lines = filein.readlines()
filein.close()
for line in lines:
  name = line[:-2]  #strip comma and newline
  tmp1 = "{" + name + ",\"" + name + "\"},\n"
  idToNameFO.write(tmp1)
idToNameFO.close()
