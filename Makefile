CXX = g++ -Wall -pedantic -std=c++11 -g
MAIN_BINARIES = $(basename $(wildcard *Main.cpp))
TEST_BINARY = ./TestAll
HEADERS = $(wildcard *.h)
OBJECTS = $(addsuffix .o, $(basename $(filter-out %Main.cpp %Test.cpp, $(wildcard *.cpp))))

.PRECIOUS: %.o
.SUFFIXES:
.PHONY: all compile test checkstyke

all: compile test checkstyle

compile: $(MAIN_BINARIES) $(TEST_BINARIES)

test: $(TEST_BINARY)
	$(TEST_BINARY)

checkstyle:
	python3 ../cpplint.py --repository=. *.h *.cpp

clean:
	rm -f *.o
	rm -f $(MAIN_BINARIES)
	rm -f $(TEST_BINARY)

%Main: %Main.o $(OBJECTS)
	$(CXX) -o $@ $^

TestAll: $(addsuffix .o, $(basename $(wildcard *Test.cpp))) $(OBJECTS)
	$(CXX) -o $@ $^ -lgtest -lgtest_main -lpthread

%.o: %.cpp $(HEADERS)
	$(CXX) -c $<
