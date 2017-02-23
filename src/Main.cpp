#include <iostream>
#include <thread>

#include "IrcConnection.h"


using std::string;


const string NICK = "NottaBot";
const string CHANNEL = "#nottabottest";


void PrintMessage(IrcConnection& connection, const IrcMessage& message);

void Authenticate(IrcConnection& connection, const IrcMessage& message);

void JoinChannel(IrcConnection& connection, const IrcMessage& message);

void SayHello(IrcConnection& connection, const IrcMessage& message);

void InterpretCommand(IrcConnection& connection, const IrcMessage& message);


int main(int argc, char** argv)
{
    IrcConnection connection("irc.freenode.net");
    connection.onMessage(PrintMessage);
    if (!connection.connect(NICK, NICK, "Definitely not a bot.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to server." << std::endl;
        std::cerr << "[0m";

        return 1;
    }

    connection.onNextMessage(Authenticate, [](const IrcMessage& message) {
        return message.prefix != NICK || message.command != "MODE";
    });

    while (connection) {
        std::this_thread::yield();
    }

    return 0;
}


void PrintMessage(IrcConnection& connection, const IrcMessage& message)
{
    std::cout << "[32m";
    if (!message.prefix.empty()) {
        std::cout << ":" << message.prefix << " ";
    }
    std::cout << message.command << " ";
    for (auto argument : message.arguments) {
        std::cout << argument << " ";
    }
    if (!message.tail.empty()) {
        std::cout << ":" << message.tail << " ";
    }
    std::cout << std::endl;
    std::cout << "[0m";
}

void Authenticate(IrcConnection& connection, const IrcMessage& message)
{
    connection.send("NickServ", "IDENTIFY hunter07");
    connection.onNextMessage(JoinChannel, [](const IrcMessage& message) {
        if (message.prefix.empty()) {
            return true;
        } 

        if (message.prefix.find("NickServ") != 0) {
            return true;
        }

        if (message.tail.find("now identified") == string::npos) {
            return true;
        } 

        return false;
    });
}

void JoinChannel(IrcConnection& connection, const IrcMessage& message)
{
    connection.join(CHANNEL);
    connection.onNextMessage(SayHello, [](const IrcMessage& message) {
        return message.command != "366" || message.arguments[1] != CHANNEL;
    });
}

void SayHello(IrcConnection& connection, const IrcMessage& message) 
{
    connection.send(CHANNEL, "Hello");
}

void InterpretCommand(IrcConnection& connection, const IrcMessage& message)
{
}
