#include <IrcConnection.h>

#include <exception>
#include <stdexcept>
#include <iostream>

#include <log.h>
#include <util.h>


using std::exception;
using std::runtime_error;
using std::string;
using std::thread;
using std::to_string;
using std::vector;
using util::to_hex;


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
    this->send(irc::message(message));
}

void IrcConnection::send(const irc::message& message)
{
    if (!this->connected()) {
        throw runtime_error("Not connected to server.");
    }
    
    string data = string(message);
    if (data.length() > 510) {
        logger::error("IrcConnection::send(message with message.size() > 510) this=0x" + to_hex((size_t)this));


        string what   = "Attempted to send message with message.size() > 510.";
        string detail = "Given message\n"
            "    " + string(message) + "\n"
            "exceeded this limit by " + to_string(data.length() - 510) + " bytes.";
        
        string suggestion;
        if (message.command() == "PRIVMSG") {
            suggestion = "Consider splitting PRIVMSG messages into multiple"
                " messages.";
        }

        throw runtime_error(what + "\n" + detail + "\n" + suggestion);
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


        irc::message message(this->messagePartial);

        logger::debug("Received message:\n    " + string(message));

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
