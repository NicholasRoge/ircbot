#include "IrcConnection.h"

#include <exception>
#include <stdexcept>
#include <iostream>


using std::exception;
using std::runtime_error;
using std::string;
using std::thread;
using std::vector;




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
    if (this->connected()) {
        this->disconnect();
    }
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
        this->sendMessage("QUIT");
    } else {
        this->sendMessage("QUIT :" + message);
    }
}

bool IrcConnection::connected() const
{
    return this->socket.connected();
}

IrcConnection::operator bool() const
{
    return this->connected();
}

void IrcConnection::sendMessage(const string& message)
{
    IrcMessage m(message);
    this->sendMessage(m);
}

void IrcConnection::sendMessage(const IrcMessage& message)
{
    if (!this->connected()) {
        throw runtime_error("Not connected to server.");
    }
    
    string data = message.toString();
    if (data.length() > 510) {
        throw runtime_error("Cannot send messages longer than 510 characters.");
    }
    this->socket.write(data);
}

void IrcConnection::setUser(const string& user, const string& phrase)
{
    this->sendMessage("USER " + user + " 0 * :" + phrase);
}

void IrcConnection::setNick(const string& nick)
{
    this->sendMessage("NICK " + nick);
}

void IrcConnection::join(const string& channel)
{
    this->sendMessage("JOIN " + channel);
}

void IrcConnection::send(const string& recipient, const string& message)
{
    this->sendMessage("PRIVMSG " + recipient + " :" + message);
}


void IrcConnection::leave(const string& channel, const string& message)
{
    if (message.empty()) {
        this->sendMessage("PART " + channel);
    } else {
        this->sendMessage("PART" + channel + " :" + message);
    }
}

void IrcConnection::whois(const string& nick)
{
    this->sendMessage("WHOIS" + nick);
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

        IrcMessage message(this->messagePartial);
        if (message.getCommand() == "PING") {
            this->sendMessage("PONG " + message.getTrailing());
        } else {
            auto iter = this->messageCallbacks.begin();
            auto end = this->messageCallbacks.end();
            while (iter != end) {
                if (iter->filter && iter->filter(message)) {
                    ++iter;

                    continue;
                }

                iter->callback(*this, message);

                if (iter->once) {
                    iter = this->messageCallbacks.erase(iter);
                } else {
                    ++iter;
                }
            }
        }

        this->messagePartial.erase(0, crlfPos + 2);
    }
}
