BUILD_DIR=bin
TARGET=ircbot
COMPILE_OPTS=--std=c++14
SOURCE=src/Main.cpp src/IrcConnection.cpp

all:
	mkdir -p "$(BUILD_DIR)"
	g++ $(COMPILE_OPTS) -o "$(BUILD_DIR)/$(TARGET)" $(SOURCE)

test: clean all
	@$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/*
