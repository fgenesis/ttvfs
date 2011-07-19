#!/bin/sh
g++ example1.cpp ../ttvfs/*.cpp -o example1 -I.. -O2 -pipe -Wall
g++ example2.cpp ../ttvfs/*.cpp -o example2 -I.. -O2 -pipe -Wall
g++ example3.cpp ../ttvfs/*.cpp -o example3 -I.. -O2 -pipe -Wall