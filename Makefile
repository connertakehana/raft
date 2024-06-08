# CC = g++
# CFLAGS = -Wall -Iinclude -std=c++20 -lzmq
# SRCS = src/main.cpp src/messege.cpp src/server.cpp src/log.cpp src/network_handler.cpp
# OBJS = $(SRCS:.cpp=.o)
# LIB = -pthread

# all: my_program

# my_program: $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -o my_program $(LIB)

# %.o: %.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) my_program

# CC = g++
# CFLAGS = -Wall -Iinclude -std=c++20 -lzmq
# SRCS = src/main.cpp src/message.cpp src/server.cpp src/log.cpp src/network_handler.cpp
# OBJS = $(SRCS:.cpp=.o)
# LIB = -pthread
# TEST_SRCS = $(wildcard tests/*.cpp)
# TEST_BINS = $(TEST_SRCS:.cpp=)

# all: my_program

# my_program: $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -o my_program $(LIB)

# tests: $(TEST_BINS)

# $(TEST_BINS): % : %.cpp
# 	$(CC) $(CFLAGS) -o $@ $< $(LIB)

# %.o: %.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS) my_program $(TEST_BINS)



CC = g++
CFLAGS = -Wall -Iinclude -std=c++20 -lzmq
SRCS = src/main.cpp src/message.cpp src/server.cpp src/log.cpp src/network_handler.cpp src/server_handler.cpp src/client_network_handler.cpp
OBJS = $(SRCS:.cpp=.o)
LIB = -pthread
TEST_SRCS = $(wildcard tests/*.cpp)
TEST_BINS = $(TEST_SRCS:.cpp=)

all: my_program tests

my_program: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o my_program $(LIB)

tests: $(TEST_BINS)

$(TEST_BINS): % : %.cpp $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(filter-out src/main.o, $(OBJS)) $(LIB)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) my_program $(TEST_BINS)
