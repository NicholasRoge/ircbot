#include <iostream>
#include <thread>

#include "IrcClient.h"


using std::string;


const string NICK = "NottaBot";

const string CHANNEL = "#nottabottest";


void PrintMessage(IrcClient& server, const IrcMessage& message);

bool IsHelpRequest(const IrcMessage& message);

void ShowHelp(IrcClient& server, const IrcMessage& message);

bool IsCommandRequest(const IrcMessage& message);

void InterpretCommand(IrcClient& server, const IrcMessage& message);

bool IrcInitialized(const IrcMessage& message);

bool Authenticate(IrcClient& server);

void JoinChannel(IrcClient& server);

void SayHello(IrcClient& server);


int main(int argc, char** argv)
{
    IrcClient server("irc.freenode.net");
    
    server.onMessage(PrintMessage);
    server.onMessage(IsHelpRequest, ShowHelp);
    server.onMessage(IsCommandRequest, InterpretCommand);

    std::cout << "Connecting to server." << std::endl;

    server.setNick(NICK);
    if (!server.connect(NICK, "Definitely not a bot.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to server." << std::endl;
        std::cerr << "[0m";

        return 1;
    }

    server.waitFor(IrcInitialized);

    std::cout << "Authenticating." << std::endl;

    if (!Authenticate(server)) {
        std::cerr << "[31m";
        std::cerr << "Failed to authenticate with NickServ." << std::endl;
        std::cerr << "[0m";

        return 2;
    }

    std::cout << "Joining channel." << std::endl;

    JoinChannel(server);

    std:: cout << "Initialized." << std::endl;

    SayHello(server);
    
    while (server) {
        std::this_thread::yield();
    }

    return 0;
}


void PrintMessage(IrcClient& server, const IrcMessage& message)
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


void ShowHelp(IrcClient& server, const IrcMessage& message)
{
    server.msg(message.getNick(), "Here's all the commands I can respond to:");
    server.msg(message.getNick(), "  !source");
    server.msg(message.getNick(), "    I will display a link to my source code.");
    server.msg(message.getNick(), "  !leave");
    server.msg(message.getNick(), "    Will cause me to leave the channel.");
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

void InterpretCommand(IrcClient& server, const IrcMessage& message)
{
    string channel = message.getArg(0);
    string trailing = message.getTrailing();
    if (trailing == "!source") {
        server.msg(channel, "Thanks for the interest!  <3");
        server.msg(channel, "You can go to https://github.com/NicholasRoge/ircbot to see my source code.");
    } else if (trailing == "!leave") {
        server.msg(channel, "Did I do something wrong...?  If I did, I'm sorry.  :c");
        server.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
    } else {
        server.msg(channel, "Sorry, I don't know how to respond to that.");
    }
}

bool Authenticate(IrcClient& server)
{
    bool authSucceeded;

    server.msg("NickServ", "IDENTIFY hunter07");
    server.waitFor([&authSucceeded](const IrcMessage& message) {
        if (message.getNick() != "NickServ") {
            return false;
        }

        string trailing = message.getTrailing();
        if (trailing.find("now identified") != string::npos) {
            authSucceeded = true;
            return true;
        } else if (trailing.find("not identified") != string::npos) {
            authSucceeded = false;
            return true;
        } else {
            return false;
        }
    });

    return authSucceeded;

}

void JoinChannel(IrcClient& server)
{
    server.join(CHANNEL);
    server.waitForResponse(366);
}

void SayHello(IrcClient& server) 
{
    server.msg(CHANNEL, "Hello.  My name is " + NICK + " and I'm definitely not a bot.");
    server.msg(CHANNEL, "If I were though, you could PM me 'help' to view the actions I'm capable of taking.");
    server.msg(CHANNEL, "It's nice to meet all of you, and I hope I can serve you well.  :D");
}
