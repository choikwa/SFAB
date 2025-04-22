# SFAB
Sentient Fabricator

a WIP build system written in python, parsing dictionary in _SFAB files as dependency. Initial implementation works on basic C++ project using static libraries.
It's not actually sentient, just chose a name for namesake.

## Goals:
- Resource-aware, parallel build: developers often have limited free memory and parallel compilation/linking fails from out of memory errors using existing build systems which do not account for system resource availability.
- Extremely simple, extensible build script that parses basically a python dictionary for dependency chain

## How-To-Use
```
cd Rekt/src
python3 ../../sfab.py
```
