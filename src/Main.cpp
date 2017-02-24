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

void ShowHelp(IrcConnection& connection, const IrcMessage& message);

bool HelpMessageFilter(const IrcMessage& message);

void InterpretCommand(IrcConnection& connection, const IrcMessage& message);


int main(int argc, char** argv)
{
    IrcConnection connection("irc.freenode.net");
    
    connection.onMessage(PrintMessage);
    connection.onMessage(ShowHelp, HelpMessageFilter);

    if (!connection.connect(NICK, NICK, "Definitely not a bot.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to server." << std::endl;
        std::cerr << "[0m";

        return 1;
    }

    connection.onNextMessage(Authenticate, [](const IrcMessage& message) {
        return message.getNick() != NICK || message.getCommand() != "MODE";
    });

    while (connection) {
        std::this_thread::yield();
    }

    return 0;
}


void PrintMessage(IrcConnection& connection, const IrcMessage& message)
{
    std::cout << "[32m";
    std::cout << message.toString();
    std::cout << "[0m";
}

void ShowHelp(IrcConnection& connection, const IrcMessage& message)
{
    connection.send(message.getNick(), "Here's all the commands I can respond to:");
    connection.send(message.getNick(), "  !source");
    connection.send(message.getNick(), "    I will display a link to my source code.");
    connection.send(message.getNick(), "  !leave");
    connection.send(message.getNick(), "    Will cause me to leave the channel.");
}

bool HelpMessageFilter(const IrcMessage& message)
{
    if (message.getCommand() != "PRIVMSG") {
         return true;
    }

    if (message.getArg(0) != NICK) {
        return true;
    }

    if (message.getTrailing() != "help") {
        return true;
    }

    return false;
}

void Authenticate(IrcConnection& connection, const IrcMessage& message)
{
    connection.send("NickServ", "IDENTIFY hunter07");
    connection.onNextMessage(JoinChannel, [](const IrcMessage& message) {
        if (message.getNick() != "NickServ") {
            return true;
        }

        if (message.getTrailing().find("now identified") == string::npos) {
            return true;
        } 

        return false;
    });
}

void JoinChannel(IrcConnection& connection, const IrcMessage& message)
{
    connection.join(CHANNEL);
    connection.onNextMessage(SayHello, [](const IrcMessage& message) {
        return message.getCommand() != "366" || message.getArg(1) != CHANNEL;
    });
}

void SayHello(IrcConnection& connection, const IrcMessage& message) 
{
    connection.send(CHANNEL, "Hello.  My name is " + NICK + " and I'm definitely not a bot.");
    connection.send(CHANNEL, "If I were though, you could PM me 'help' to view the actions I'm capable of taking.");
    connection.send(CHANNEL, "It's nice to meet all of you, and I hope I can serve you well.  :D");
    connection.onMessage(InterpretCommand, [](const IrcMessage& message) {
        if (message.getCommand() != "PRIVMSG") {
            return true;
        }

        if (message.getArg(0) != CHANNEL) {
            return true;
        }

        if (message.getTrailing()[0] != '!') {
            return true;
        }

        return false;
    });
}

void InterpretCommand(IrcConnection& connection, const IrcMessage& message)
{
    string channel = message.getArg(0);
    string trailing = message.getTrailing();
    if (trailing == "!source") {
        connection.send(channel, "Thanks for the interest!  <3");
        connection.send(channel, "You can go to https://github.com/NicholasRoge/ircbot to see my source code.");
    } else if (trailing == "!leave") {
        connection.send(channel, "Did I do something wrong...?  If I did, I'm sorry.  :c");
        connection.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
    } else {
        connection.send(channel, "Sorry, I don't know how to respond to that.");
    }
}
