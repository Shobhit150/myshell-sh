CXX = g++
CXXFLAGS = -std=c++23 -O2

SRC := $(shell find src -name "*.cpp")
OBJ := $(SRC:.cpp=.o)

shell: $(OBJ)
	$(CXX) $(OBJ) -o shell

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) shell