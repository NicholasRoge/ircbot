#include "IrcConnection.h"

#include <exception>
#include <stdexcept>
#include <iostream>


using std::exception;
using std::runtime_error;
using std::string;
using std::thread;
using std::vector;




IrcConnection::IrcConnection()
{
    this->socket.onData([this](void* data, size_t byteCount) {
        this->onData(data, byteCount);
    });
}

IrcConnection::~IrcConnection()
{
    this->disconnect();
}

bool IrcConnection::connect(const string& url, unsigned short port)
{
    if (this->socket) {
        return true;
    }

    if (!this->socket.connectTo(url, port)) {
        return false;
    }

    this->messagePartial = "";

    return true;
}

void IrcConnection::disconnect()
{
    if (!this->connected()) {
        return;
    }
    
    this->socket.disconnect();
}

bool IrcConnection::connected() const
{
    return this->socket.connected();
}

IrcConnection::operator bool() const
{
    return this->connected();
}

void IrcConnection::send(const string& message)
{
    IrcMessage m(message);
    this->send(m);
}

void IrcConnection::send(const IrcMessage& message)
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

void IrcConnection::onMessage(MessageCallback callback, MessageFilter filter)
{
    MessageCallbackObject mco;
    mco.callback = callback;
    mco.filter = filter;
    mco.once = false;
    this->messageCallbacks.push_back(mco);
}

void IrcConnection::onNextMessage(MessageCallback callback, MessageFilter filter)
{
    MessageCallbackObject mco;
    mco.callback = callback;
    mco.filter = filter;
    mco.once = true;
    this->messageCallbacks.push_back(mco);
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

        this->messagePartial.erase(0, crlfPos + 2);
    }
}
