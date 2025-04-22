# author: thinkdoge

CC=clang++
CFLAGS=-Wall -Wextra -g -c -std=c++17
LDFLAGS=
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)


all: rekt

rekt: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@
	
depend: .depend

.depend: $(SRCS) idToNameMap.txt
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend
	
idToNameMap.txt: Lexemes.txt genTokenNameMap.py
	python3 genTokenNameMap.py

include .depend

clean:
	rm -f $(LIB) $(OBJS) .depend idToNameMap.txt nameToIdMap.txt rekt
	touch Lexemes.txt
	
.PHONY: clean
