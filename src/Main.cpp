#include <iostream>
#include <stdio.h>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <vector>

#include <IrcClient.h>


using std::string;
using std::vector;


const string NICK = "NottaBot";

termios initial_term_settings;


void PrintIncoming(const string& source, const string& message);

void PrintOutgoing(const string& recipient, const string& message);

bool IsChannelMention(const irc::message& message);

bool PrintChannelMessage(IrcClient& client, const irc::message& message);

bool IsPrivateMessage(const irc::message& message);

bool PrintPrivateMessage(IrcClient& client, const irc::message& message);

bool IsHelpRequest(const irc::message& message);

void ShowHelp(IrcClient& client, const irc::message& message);

bool IsCommandRequest(const irc::message& message);

void InterpretCommand(IrcClient& client, const irc::message& message);

void InitTerminal();

void DeinitTerminal();

void InitClient(IrcClient& client);

bool IrcInitialized(const irc::message& message);

void JoinChannel(IrcClient& client, const string& channel);

void SayHello(IrcClient& client, const string& channel);


int main(int argc, char** argv, char** watv)
{
    InitTerminal();

    IrcClient client("irc.freenode.net");
    InitClient(client);
    for (int arg = 1; arg < argc;++arg) {
        JoinChannel(client, argv[arg]);
    }

    while (client) {
        char c = getchar();
        if (c == 'q') {
            break;
        }
    }

    client.disconnect();

    DeinitTerminal();

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

bool IsChannelMention(const irc::message& message)
{
    return message.command() == "PRIVMSG" && message.nick() != NICK && message.tail().find(NICK) != string::npos;
}

bool PrintChannelMessage(IrcClient& client, const irc::message& message)
{
    PrintIncoming("[" + message.arg(0) + "]" + message.nick(), message.tail());
}

bool IsPrivateMessage(const irc::message& message)
{
    return message.command() == "PRIVMSG" && message.arg(0) == NICK;
}

bool PrintPrivateMessage(IrcClient& client, const irc::message& message)
{
    PrintIncoming(message.nick(), message.tail());
}

bool IsHelpRequest(const irc::message& message)
{
    if (message.command() != "PRIVMSG") {
         return false;
    }

    if (message.arg(0) != NICK) {
        return false;
    }

    if (message.tail() != "help") {
        return false;
    }

    return true;
}

void ShowHelp(IrcClient& client, const irc::message& message)
{
    static const vector<string> LINES = {
        "Here's all the commands I can respond to:",
        "  !source",
        "    I will display a link to my source code.",
        "  !leave (restricted:  channel operators only)",
        "    Will cause me to leave the channel."
    };

    for (auto& line : LINES) {
        PrintOutgoing(message.nick(), line);
        client.msg(message.nick(), line);
    }
}

bool IsCommandRequest(const irc::message& message)
{
    if (message.command() != "PRIVMSG") {
        return false;
    }

    if (message.nick() == NICK || message.arg(0) == NICK) {
        return false;
    }

    if (message.tail()[0] != '!') {
        return false;
    }

    return true;
}

void InterpretCommand(IrcClient& client, const irc::message& message)
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

    string channel = message.arg(0);
    string trailing = message.tail();
    if (trailing == "!source") {
        for (auto& line : SOURCE_LINES) {
            PrintOutgoing("[" + message.arg(0) + "]", line);
            client.msg(channel, line);
        }
    } else if (trailing == "!leave") {
        client.whois(message.nick());
        client.onNextResponse(319, [channel](IrcClient& client, const irc::message& message) {
            auto pos = message.tail().find(channel);
            if (pos != 0 && message.tail()[pos - 1] == '@') {
                for (auto& line : LEAVE_LINES_OP) {
                    PrintOutgoing("[" + message.arg(0) + "]", line);
                    client.msg(channel, line);
                }
                client.leave(channel, "Have a nice day everyone!  I'll see you again soon.");
            } else {
                for (auto& line : LEAVE_LINES_NONOP) {
                    PrintOutgoing("[" + message.arg(0) + "]", line);
                    client.msg(channel, line);
                }
            }
        });
    } else {
        for (auto& line : UNKNOWN_COMMAND_LINES) {
            PrintOutgoing("[" + message.arg(0) + "]", line);
            client.msg(channel, line);
        }
    }
}

void InitTerminal()
{
    termios settings;

    tcgetattr(STDIN_FILENO, &initial_term_settings);
    settings = initial_term_settings;
    settings.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

void DeinitTerminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &initial_term_settings);
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

bool IrcInitialized(const irc::message& message)
{
    return message.command() == "MODE" && message.arg(0) == NICK;
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
