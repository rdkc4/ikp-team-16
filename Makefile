CXX = clang++
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -g
SANITIZERS = -fsanitize=address,undefined

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = gcsim

$(EXEC): $(OBJ)
	$(CXX) $(SANITIZERS) -o $(EXEC) $(OBJ)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(SANITIZERS) $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
