run : RUSH
	./RUSH num.bin
RUSH : project2.cpp
	g++ -std=c++11 project2.cpp -o RUSH
