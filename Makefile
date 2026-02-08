CXX = clang++
CXXFLAGS = -std=c++23 -g -O1 -pthread \
           -Wall -Wextra -Wpedantic -Werror \
           -fno-omit-frame-pointer \
           -fno-optimize-sibling-calls \
           -D_GLIBCXX_ASSERTIONS

SANITIZERS = -fsanitize=address,undefined

SRC = main.cpp \
	src/common/header/header.cpp \
	src/common/segment/segment-info.cpp \
	src/common/segment/segment.cpp \
	src/common/thread-pool/thread-pool.cpp \
	src/heap/heap.cpp \
	src/root-set-table/global-root.cpp \
	src/root-set-table/register-root.cpp \
	src/root-set-table/thread-local-stack.cpp \
	src/root-set-table/root-set-table.cpp \
	src/segment-free-memory-table/segment-free-memory-table.cpp \
	src/garbage-collector/gc.cpp \
	src/heap-manager/heap-manager.cpp \
	src/allocators/allocators.cpp

OBJ = $(SRC:.cpp=.o)
EXEC = gcsim

$(EXEC): $(OBJ)
	$(CXX) $(SANITIZERS) -o $(EXEC) $(OBJ)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(SANITIZERS) $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
