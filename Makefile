CXX = clang++
CXXFLAGS = -std=c++23 -fsanitize=address,undefined -Wall -Wextra -Werror -g

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = gcsim

$(EXEC): $(OBJ)
	$(CXX) -o $(EXEC) $(OBJ)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
