#include <iostream>
#include <thread>

#include "IrcClient.h"


using std::string;


const string NICK = "NottaBot";

const string CHANNEL = "#nottabottest";


void PrintMessage(IrcClient& client, const IrcMessage& message);

bool IsHelpRequest(const IrcMessage& message);

void ShowHelp(IrcClient& client, const IrcMessage& message);

bool IsCommandRequest(const IrcMessage& message);

void InterpretCommand(IrcClient& client, const IrcMessage& message);

bool IrcInitialized(const IrcMessage& message);

void JoinChannel(IrcClient& client);

void SayHello(IrcClient& client);


int main(int argc, char** argv)
{
    IrcClient client("irc.freenode.net");
    
    client.onMessage(PrintMessage);
    client.onMessage(IsHelpRequest, ShowHelp);
    client.onMessage(IsCommandRequest, InterpretCommand);

    std::cout << "Connecting to server." << std::endl;

    client.setNick(NICK, "hunter07");
    if (!client.connect(NICK, "Am I a bot?  Certainly not.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to client." << std::endl;
        std::cerr << "[0m";

        return 1;
    }

    client.waitFor(IrcInitialized);

    std::cout << "Joining channel." << std::endl;

    JoinChannel(client);

    std:: cout << "Initialized." << std::endl;

    SayHello(client);
    
    while (client) {
        std::this_thread::yield();
    }

    return 0;
}


void PrintMessage(IrcClient& client, const IrcMessage& message)
{
    std::cout << "[32m";
    std::cout << message.toString();
    std::cout << "[0m";
}

bool IsHelpRequest(const IrcMessage& message)
{
    if (message.getCommand() != "PRIVMSG") {
         return false;
    }

    if (message.getArg(0) != NICK) {
        return false;
    }

    if (message.getTrailing() != "help") {
        return false;
    }

    return true;
}


void ShowHelp(IrcClient& client, const IrcMessage& message)
{
    client.msg(message.getNick(), "Here's all the commands I can respond to:");
    client.msg(message.getNick(), "  !source");
    client.msg(message.getNick(), "    I will display a link to my source code.");
    client.msg(message.getNick(), "  !leave");
    client.msg(message.getNick(), "    Will cause me to leave the channel.");
}

bool IsCommandRequest(const IrcMessage& message)
{
    if (message.getCommand() != "PRIVMSG") {
        return false;
    }

    if (message.getArg(0) != CHANNEL) {
        return false;
    }

    if (message.getTrailing()[0] != '!') {
        return false;
    }

    return true;
}

bool IrcInitialized(const IrcMessage& message)
{
    return message.getCommand() == "MODE" && message.getArg(0) == NICK;
}

void InterpretCommand(IrcClient& client, const IrcMessage& message)
{
    string channel = message.getArg(0);
    string trailing = message.getTrailing();
    if (trailing == "!source") {
        client.msg(channel, "Thanks for the interest!  <3");
        client.msg(channel, "You can go to https://github.com/NicholasRoge/ircbot to see my source code.");
    } else if (trailing == "!leave") {
        client.msg(channel, "Did I do something wrong...?  If I did, I'm sorry.  :c");
        client.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
    } else {
        client.msg(channel, "Sorry, I don't know how to respond to that.");
    }
}

void JoinChannel(IrcClient& client)
{
    client.join(CHANNEL);
    client.waitForResponse(366);
}

void SayHello(IrcClient& client) 
{
    client.msg(CHANNEL, "Hello.  My name is " + NICK + " and I'm definitely not a bot.");
    client.msg(CHANNEL, "If I were though, you could PM me 'help' to view the actions I'm capable of taking.");
    client.msg(CHANNEL, "It's nice to meet all of you, and I hope I can serve you well.  :D");
}
