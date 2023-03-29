CC = g++
CFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -pthread -lssl -lcrypto

SERVER_SRC = server.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_BIN = server

CLIENT_SRC = client.cpp
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
CLIENT_BIN = client

.PHONY: all clean

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(LDFLAGS) -o $@

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN) $(SERVER_OBJ) $(CLIENT_OBJ)
