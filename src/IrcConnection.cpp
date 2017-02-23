#include "IrcConnection.h"

#include <exception>
#include <stdexcept>
#include <iostream>


using std::exception;
using std::runtime_error;
using std::string;
using std::thread;
using std::vector;


IrcMessage IrcMessage::parse(string s)
{
    size_t offset = 0;

    IrcMessage message;

    if (s[0] == ':') {
        message.prefix = IrcMessage::nextWord(s);
        message.prefix.erase(0, 1);
    }

    message.command = IrcMessage::nextWord(s);
    
    while (!s.empty()) {
        if (s[0] == ':') {
            message.tail = s;
            message.tail.erase(0, 1);

            break;
        } else {
            message.arguments.push_back(IrcMessage::nextWord(s));
        }
    }

    return message;
}

string IrcMessage::nextWord(string& s)
{
    auto end = s.find(" ");
    if (end == string::npos) {
        string word(s);
        s.clear();
        return word;
        throw runtime_error("Malformed irc message.");
    } else {
        string word(s.begin(), s.begin() + end);
        s.erase(0, end + 1);
        return word;
    }

}


IrcConnection::IrcConnection(const string& url, unsigned short port)
{
    this->url = url;
    this->port = port;
    this->messagePartial = "";

    this->socket.onData([this](void* data, size_t byteCount) {
        this->onData(data, byteCount);
    });
}

IrcConnection::~IrcConnection()
{
    this->disconnect();
}

bool IrcConnection::connect(const string& nick, const string& user, const string& userComment)
{
    if (this->socket) {
        return true;
    }

    if (!this->socket.connectTo(this->url, this->port)) {
        return false;
    }

    this->setUser(user, userComment);
    this->setNick(nick);

    return true;
}

void IrcConnection::disconnect(const string& message)
{
    if (message.empty()) {
        this->command("QUIT");
    } else {
        this->command("QUIT", ":" + message);
    }
    this->socket.disconnect();
    this->messagePartial = "";
}

bool IrcConnection::connected() const
{
    return this->socket.connected();
}

IrcConnection::operator bool() const
{
    return this->connected();
}

void IrcConnection::command(const string& command, const string& args)
{
    string data = command;
    if (!args.empty()) {
        data += " " + args;
    }
    data += "\r\n";

    if (data.length() > 510) {
        throw runtime_error("Cannot send messages longer than 510 characters.");
    }

    std::cout << "[37m" << data << "[0m";
    try {
        this->socket.write(data);
    } catch (exception& e) {
        std::cerr << "[31m";
        std::cerr << "Failed to send message.  Cause:" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "[0m";
    }
}

void IrcConnection::setUser(const string& user, const string& phrase)
{
    this->command("USER", user + " 0 * :" + phrase);
}

void IrcConnection::setNick(const string& nick)
{
    this->command("NICK", nick);
}

void IrcConnection::join(const string& channel)
{
    this->command("JOIN", channel);
}

void IrcConnection::send(const string& recipient, const string& message)
{
    this->command("PRIVMSG", recipient + " :" + message);
}


void IrcConnection::leave(const string& channel, const string& message)
{
    if (message.empty()) {
        this->command("PART", channel);
    } else {
        this->command("PART", channel + " :" + message);
    }
}

void IrcConnection::whois(const string& nick)
{
    this->command("WHOIS", nick);
}

void IrcConnection::onMessage(MessageCallback callback, MessageFilter filter)
{
    this->messageCallbacks.push_back(MessageCallbackObject{
        callback,
        filter,
        false
    });
}

void IrcConnection::onNextMessage(MessageCallback callback, MessageFilter filter)
{
    this->messageCallbacks.push_back(MessageCallbackObject{
        callback,
        filter,
        true
    });
}

void IrcConnection::onData(void* data, size_t byteCount)
{
    this->messagePartial.append((char*)data, byteCount);

    while (true) {
        auto crlfPos = this->messagePartial.find("\r\n");
        if (crlfPos == string::npos) {
            break;
        }

        auto messageString = this->messagePartial.substr(0, crlfPos);
        if (!messageString.empty()) {
            auto message = IrcMessage::parse(messageString);
            if (message.command == "PING") {
                this->command("PONG", message.tail);
            } else {
                auto iter = this->messageCallbacks.begin();
                auto end = this->messageCallbacks.end();
                while (iter != end) {
                    if (iter->filter && iter->filter(message)) {
                        std::cout << "[33m";
                        std::cout << "Filtered callback." << std::endl;
                        std::cout << "[0m";

                        ++iter;

                        continue;
                    }

                    iter->callback(*this, message);

                    if (iter->once) {
                        iter = this->messageCallbacks.erase(iter);

                        std::cout << "[33m";
                        std::cout << "Removed callback." << std::endl;
                        std::cout << "[0m";
                    } else {
                        ++iter;
                    }
                }
            }
        }
        this->messagePartial.erase(0, crlfPos + 2);
    }
}
