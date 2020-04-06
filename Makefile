CXX=g++
CXXFLAGS= -Wall -g 
MAIN= ##your_source_code_name

first_fit.o: mem_manager.cpp mem_manager.h
	$(CXX) $(CXXFLAGS) -c -o first_fit.o mem_manager.cpp

best_fit.o: mem_manager.cpp mem_manager.h
	$(CXX) $(CXXFLAGS) -c -DPOLICY -o best_fit.o mem_manager.cpp

first_fit: main.o first_fit.o
	$(CXX) $(CXXFLAGS) -o first_fit main.o first_fit.o

best_fit: main.o best_fit.o
	$(CXX) $(CXXFLAGS) -o best_fit main.o best_fit.o


main.o: $(MAIN).cpp mem_manager.h

mem_manager.o: mem_manager.h


