BUILD_DIR=bin
TARGET=ircbot
STD=c++14
CFLAGS=--std=$(STD)
LIBS=-pthread
SOURCE=src/Main.cpp src/IrcMessage.cpp src/IrcConnection.cpp src/Socket.cpp

all:
	mkdir -p "$(BUILD_DIR)"
	g++ $(CFLAGS)\
		$(LIBS)\
	   	-o "$(BUILD_DIR)/$(TARGET)"\
	   	$(SOURCE)

debug: CFLAGS:=$(CFLAGS) -g -DDEBUG
debug: all

clean:
	rm -rf $(BUILD_DIR)/*

