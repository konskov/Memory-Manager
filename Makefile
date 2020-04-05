CXX=g++
CXXFLAGS= -Wall -g


main: main.o mem_manager.o
	$(CXX) $(CXXFLAGS) -o main main.o mem_manager.o

main.o: main.cpp mem_manager.h

mem_manager.o: mem_manager.h
