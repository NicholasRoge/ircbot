#include <iostream>
#include <thread>
#include <vector>

#include "IrcClient.h"


using std::string;
using std::vector;


const string NICK = "NottaBot";


void PrintIncoming(const string& source, const string& message);

void PrintOutgoing(const string& recipient, const string& message);

bool IsChannelMention(const IrcMessage& message);

bool PrintChannelMessage(IrcClient& client, const IrcMessage& message);

bool IsPrivateMessage(const IrcMessage& message);

bool PrintPrivateMessage(IrcClient& client, const IrcMessage& message);

bool IsHelpRequest(const IrcMessage& message);

void ShowHelp(IrcClient& client, const IrcMessage& message);

bool IsCommandRequest(const IrcMessage& message);

void InterpretCommand(IrcClient& client, const IrcMessage& message);

void InitClient(IrcClient& client);

bool IrcInitialized(const IrcMessage& message);

void JoinChannel(IrcClient& client, const string& channel);

void SayHello(IrcClient& client, const string& channel);


int main(int argc, char** argv)
{
    IrcClient client("irc.freenode.net");
    InitClient(client);
    for (int arg = 1; arg < argc;++arg) {
        JoinChannel(client, argv[arg]);
    }
    while (client) {
        std::this_thread::yield();
    }

    return 0;
}


void PrintIncoming(const string& source, const string& message)
{
    std::cout << source << " => " << NICK << ":  " << message << std::endl;
}

void PrintOutgoing(const string& recipient, const string& message)
{
    std::cout << NICK << " => " << recipient << ":  " << message << std::endl;
}

bool IsChannelMention(const IrcMessage& message)
{
    return message.getCommand() == "PRIVMSG" && message.getNick() != NICK && message.getTrailing().find(NICK) != string::npos;
}

bool PrintChannelMessage(IrcClient& client, const IrcMessage& message)
{
    PrintIncoming("[" + message.getArg(0) + "]" + message.getNick(), message.getTrailing());
}

bool IsPrivateMessage(const IrcMessage& message)
{
    return message.getCommand() == "PRIVMSG" && message.getArg(0) == NICK;
}

bool PrintPrivateMessage(IrcClient& client, const IrcMessage& message)
{
    PrintIncoming(message.getNick(), message.getTrailing());
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
    static const vector<string> LINES = {
        "Here's all the commands I can respond to:",
        "  !source (restricted:  channel operators only)",
        "    I will display a link to my source code.",
        "  !leave",
        "    Will cause me to leave the channel."
    };

    for (auto& line : LINES) {
        PrintOutgoing(message.getNick(), line);
        client.msg(message.getNick(), line);
    }
}

bool IsCommandRequest(const IrcMessage& message)
{
    if (message.getCommand() != "PRIVMSG") {
        return false;
    }

    if (message.getNick() == NICK || message.getArg(0) == NICK) {
        return false;
    }

    if (message.getTrailing()[0] != '!') {
        return false;
    }

    return true;
}

void InterpretCommand(IrcClient& client, const IrcMessage& message)
{
    static const vector<string> SOURCE_LINES = {
        "Thanks for the interest!  <3",
        "You can go to http://github.com/NicholasRoge/ircbot to see my source code."
    };
    static const vector<string> LEAVE_LINES_OP = {
        "If I did something wrong, I sincerely apologize.",
        "Please report any issues to my maintainer at https://github.com/NicholasRoge/ircbot."
    };
    static const vector<string> LEAVE_LINES_NONOP = {
        "I'm sorry, only channel operators may use that command."
    };
    static const vector<string> UNKNOWN_COMMAND_LINES = {
        "I'm sorry, I don't know that command."
    };

    string channel = message.getArg(0);
    string trailing = message.getTrailing();
    if (trailing == "!source") {
        for (auto& line : SOURCE_LINES) {
            PrintOutgoing("[" + message.getArg(0) + "]", line);
            client.msg(channel, line);
        }
    } else if (trailing == "!leave") {
        client.whois(message.getNick());
        client.onNextResponse(319, [channel](IrcClient& client, const IrcMessage& message) {
            auto pos = message.getTrailing().find(channel);
            if (pos != 0 && message.getTrailing()[pos - 1] == '@') {
                for (auto& line : LEAVE_LINES_OP) {
                    PrintOutgoing("[" + message.getArg(0) + "]", line);
                    client.msg(channel, line);
                }
                client.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
            } else {
                for (auto& line : LEAVE_LINES_NONOP) {
                    PrintOutgoing("[" + message.getArg(0) + "]", line);
                    client.msg(channel, line);
                }
            }
        });
    } else {
        for (auto& line : UNKNOWN_COMMAND_LINES) {
            PrintOutgoing("[" + message.getArg(0) + "]", line);
            client.msg(channel, line);
        }
    }
}

void InitClient(IrcClient& client)
{
    client.onMessage(IsPrivateMessage, PrintPrivateMessage);
    client.onMessage(IsChannelMention, PrintChannelMessage);
    client.onMessage(IsCommandRequest, PrintChannelMessage);

    client.onMessage(IsHelpRequest, ShowHelp);
    client.onMessage(IsCommandRequest, InterpretCommand);

    std::cout << "\x1B[1;30m";
    std::cout << "Connecting to server." << std::endl;
    std::cout << "\x1B[0m";

    client.setNick(NICK, "hunter07");
    if (!client.connect(NICK, "Am I a bot?  Most decidedly not.")) {
        std::cerr << "[31m";
        std::cerr << "Failed to connect to client." << std::endl;
        std::cerr << "[0m";

        return;
    }

    client.waitFor(IrcInitialized);

    std::cout << "\x1B[1;30m";
    std::cout << "Initialized." << std::endl;
    std::cout << "\x1B[0m";
}

bool IrcInitialized(const IrcMessage& message)
{
    return message.getCommand() == "MODE" && message.getArg(0) == NICK;
}

void JoinChannel(IrcClient& client, const string& channel)
{
    std::cout << "\x1B[1;30m";
    std::cout << "Joining [" << channel << "]." << std::endl;
    std::cout << "\x1B[0m";

    client.join(channel);
    client.waitForResponse(366);
    SayHello(client, channel);
}

void SayHello(IrcClient& client, const string& channel) 
{
    static const vector<string> LINES {
        "Hello.  My name is " + NICK + " and I'm definitely not a bot.",
        "If I were though, you could PM me 'help' to view the actions I'm capable of taking.",
        "It's nice to meet all of you, and I hope I can serve you well.  :D"
    };

    std::cout << "\x1B[1;30m";
    std::cout << "Saying hello." << std::endl;
    std::cout << "\x1B[0m";
    for (auto& line : LINES) {
        PrintOutgoing("[" + channel + "]", line);
        client.msg(channel, line);
    }

}
