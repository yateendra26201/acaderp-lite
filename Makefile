CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -fno-exceptions
SOURCES = src/main.cpp src/input.cpp src/core.cpp src/advanced.cpp

.PHONY: all clean test
all: acaderp

acaderp: $(SOURCES) src/erp.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@

clean:
	rm -f acaderp

test: acaderp
	./run_tests.sh
