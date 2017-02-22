#include <iostream>
#include <thread>

#include "IrcConnection.h"


using std::string;


const string NICK = "NottaBot";

IrcConnection::MessageCallback messageHandler;


void PrintMessage(IrcConnection& connection, const IrcMessage& message);

void MessageHandlerProxy(IrcConnection& connection, const IrcMessage& message);

void OnStartupMessage(IrcConnection& connection, const IrcMessage& message);

void OnIdentifiedMessage(IrcConnection& connection, const IrcMessage& message);

void OnJoiningMessage(IrcConnection& connection, const IrcMessage& message);

void OnChannelMessage(IrcConnection& connection, const IrcMessage& message);


int main(int argc, char** argv)
{
    messageHandler = OnStartupMessage;

    IrcConnection connection("irc.freenode.net");
    connection.onMessage(PrintMessage);
    connection.onMessage(MessageHandlerProxy);
    if (!connection.connect(NICK, NICK, "Definitely not a bot.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to server." << std::endl;
        std::cerr << "[0m";

        return 1;
    }

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

void MessageHandlerProxy(IrcConnection& connection, const IrcMessage& message)
{
    messageHandler(connection, message);
}

void OnStartupMessage(IrcConnection& connection, const IrcMessage& message)
{
    if (message.prefix != NICK) {
        return;
    }

    if (message.command != "MODE") {
        return;
    }

    messageHandler = OnIdentifiedMessage;
    connection.send("NickServ", "IDENTIFY hunter07");
}

void OnIdentifiedMessage(IrcConnection& connection, const IrcMessage& message)
{
    if (message.prefix.empty()) {
        return;
    } 

    if (message.prefix.find("NickServ") != 0) {
        return;
    }

    if (message.tail.find("now identified") == string::npos) {
        return;
    } 

    messageHandler = OnJoiningMessage;
    connection.join("#freenode");
}

void OnJoiningMessage(IrcConnection& connection, const IrcMessage& message) 
{
    if (message.command != "328") {
        return;
    }

    messageHandler = OnJoiningMessage;
    connection.send("#freenode", "[Time Neutral Greeting] everyone!  If you're reading this, it means Keytap got his bot working and would like to extend his thanks to raccoon and password2 for the help.");
    connection.whois(NICK);
}

void OnChannelMessage(IrcConnection& connection, const IrcMessage& message)
{
}
