#include <iostream>
#include <thread>

#include "IrcConnection.h"


using std::string;


const string NICK = "NottaBot";
const string CHANNEL = "#laravel";


void PrintMessage(IrcConnection& connection, const IrcMessage& message);

void Authenticate(IrcConnection& connection, const IrcMessage& message);

void JoinChannel(IrcConnection& connection, const IrcMessage& message);

void SayHello(IrcConnection& connection, const IrcMessage& message);

void ShowHelp(IrcConnection& connection, const IrcMessage& message);

void InterpretCommand(IrcConnection& connection, const IrcMessage& message);


int main(int argc, char** argv)
{
    IrcConnection connection("irc.freenode.net");
    connection.onMessage(PrintMessage);
    connection.onMessage(ShowHelp, [](const IrcMessage& message) {
        if (message.command != "PRIVMSG") {
             return true;
        }

        if (message.arguments[0] != NICK) {
            return true;
        }

        if (message.tail != "help") {
            return true;
        }

        return false;
    });
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
    connection.send(CHANNEL, "Hello.  My name is " + NICK + " and I'm definitely not a bot.");
    connection.send(CHANNEL, "If I were though, you could PM me 'help' to view the actions I'm capable of taking.");
    connection.send(CHANNEL, "It's nice to meet all of you, and I hope I can serve you well.  :D");
    connection.onMessage(InterpretCommand, [](const IrcMessage& message) {
        if (message.command != "PRIVMSG") {
            return true;
        }

        if (message.arguments[0] != CHANNEL) {
            return true;
        }

        if (message.tail[0] != '!') {
            return true;
        }

        return false;
    });
}

void ShowHelp(IrcConnection& connection, const IrcMessage& message)
{
    auto end = message.prefix.find("!");
    string target = message.prefix.substr(0, end);
    connection.send(target, "Here's all the commands I can respond to:");
    connection.send(target, "  !source");
    connection.send(target, "    I will display a link to my source code.");
    connection.send(target, "  !leave");
    connection.send(target, "    Will cause me to leave the channel.");
}

void InterpretCommand(IrcConnection& connection, const IrcMessage& message)
{
    string channel = message.arguments[0];
    if (message.tail == "!source") {
        connection.send(channel, "Thanks for the interest!  <3");
        connection.send(channel, "You can go to https://github.com/NicholasRoge/ircbot to see my source code.");
    } else if (message.tail == "!leave") {
        connection.send(channel, "Did I do something wrong...?  If I did, I'm sorry.  :c");
        connection.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
    } else {
        connection.send(channel, "Sorry, I don't know how to respond to that.");
    }
}
