#include "IrcClient.h"

#include <thread>

#include "InterruptedException.h"


using std::runtime_error;
using std::string;
using std::to_string;


string ResponseToCommand(unsigned short response);


string ResponseToCommand(unsigned short response) 
{
    string strRep = to_string(response);
    switch (strRep.length()) {  // Quick and dirty but it gets the job done 
    case 1:
        strRep = "00" + strRep;
        break;

    case 2:
        strRep = "0" + strRep;
        break;

    case 3:
        break;

    default:
        throw runtime_error("Response codes may not be longer than three digits.");
    }
    return strRep;
}

MessageCallbackTrigger operator&&(const MessageCallbackTrigger& lop, const MessageCallbackTrigger& rop)
{
    return [lop, rop](const IrcMessage& message) {
        return lop(message) && rop(message);
    };
}

MessageCallbackTrigger operator||(const MessageCallbackTrigger& lop, const MessageCallbackTrigger& rop)
{
    return [lop, rop](const IrcMessage& message) {
        return lop(message) || rop(message);
    };
}

IrcClient::IrcClient(const std::string& url, unsigned short port)
{
    this->url = url;
    this->port = port;

    this->onCommand("PING", [](IrcClient& server, const IrcMessage& message) {
        server.connection.send("PONG :" + message.getTrailing());
    });
}

IrcClient::~IrcClient()
{
    if (this->connection) {
        this->disconnect();
    }
}

bool IrcClient::connect(const string& user, const string& realName, int mode)
{
    if (this->connection) {
        return true;
    }

    if (!this->connection.connect(this->url, this->port)) {
        return false;
    }

    this->setUser(user, realName, mode);
    if (this->nick.empty()) {
        this->setNick(user);
    } else {
        this->setNick(this->nick);
    }
    this->waitForResponse(1);

    return true;
}

void IrcClient::disconnect(const string& message)
{
    if (message.empty()) {
        this->connection.send("QUIT");
    } else {
        this->connection.send("QUIT :" + message);
    }
}

bool IrcClient::connected() const
{
    return this->connection.connected();
}

IrcClient::operator bool() const
{
    return this->connected();
}

void IrcClient::onMessage(const MessageCallback& callback)
{
    this->connection.onMessage(
        [this, callback](IrcConnection& connection, const IrcMessage& message) {
            callback(*this, message);
        }
    );
}

void IrcClient::onMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    // Temporary until I get around to reworking the IrcConnection interface
    // as well
    this->connection.onMessage(
        [this, callback](IrcConnection& connection, const IrcMessage& message) {
            callback(*this, message);
        },
        [trigger](const IrcMessage& message) {
            return !trigger(message);
        }
   );
}

void IrcClient::onNextMessage(const MessageCallback& callback)
{
    this->connection.onNextMessage(
        [this, callback](IrcConnection& connection, const IrcMessage& message) {
            callback(*this, message);
        }
    );
}

void IrcClient::onNextMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    // Temporary until I get around to reworking the IrcConnection interface
    // as well
    this->connection.onNextMessage(
        [this, callback](IrcConnection& connection, const IrcMessage& message) {
            callback(*this, message);
        },
        [trigger](const IrcMessage& message) {
            return !trigger(message);
        }
   );
};

void IrcClient::onCommand(const std::string& command, const MessageCallback& callback)
{
    this->onMessage(
        [command](const IrcMessage& message) {
            return message.getCommand() == command;
        },
        callback
    );
}

void IrcClient::onCommand(const std::string& command, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onMessage(
        [command](const IrcMessage& message) {
            return message.getCommand() == command;
        } && trigger,
        callback
    );
}

void IrcClient::onNextCommand(const std::string& command, const MessageCallback& callback)
{
    this->onNextMessage(
        [command](const IrcMessage& message) {
            return message.getCommand() == command;
        },
        callback
    );
}

void IrcClient::onNextCommand(const std::string& command, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onNextMessage(
        [command](const IrcMessage& message) {
            return message.getCommand() == command;
        } && trigger,
        callback
    );
}

void IrcClient::onResponse(unsigned short code, const MessageCallback& callback)
{
    this->onCommand(ResponseToCommand(code), callback); 
}

void IrcClient::onResponse(unsigned short code, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onCommand(ResponseToCommand(code), trigger, callback); 
}

void IrcClient::onNextResponse(unsigned short code, const MessageCallback& callback)
{
    this->onNextCommand(ResponseToCommand(code), callback); 
}

void IrcClient::onNextResponse(unsigned short code, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onNextCommand(ResponseToCommand(code), trigger, callback); 
}

void IrcClient::waitFor(const MessageCallbackTrigger& trigger)
{
    bool block = true;
    
    this->onNextMessage(
        trigger,
        [&block](IrcClient& server, const IrcMessage& message) {
            block = false;
        }
    );
    
    while (block) {
        if (!this->connection) {
            throw InterruptedException();
        }
        std::this_thread::yield();
    }
}

void IrcClient::waitForCommand(const string& command)
{
    this->waitFor([command](const IrcMessage& message) {
        return message.getCommand() == command;
    });
}

void IrcClient::waitForResponse(unsigned short code)
{
    this->waitForCommand(ResponseToCommand(code));
}

void IrcClient::setUser(const string& user, const string& realName, int mode)
{
    IrcMessage message;
    message.setCommand("USER");
    message.appendArg(user);
    message.appendArg(to_string(mode));
    message.appendArg("*");
    message.setTrailing(realName);
    this->connection.send(message);
}

void IrcClient::setNick(const string& nick)
{
    this->nick = nick;
    if (this->connection) {
        this->connection.send("NICK " + nick);
    }
}

void IrcClient::join(const string& channel)
{
    this->connection.send("JOIN " + channel);
}

void IrcClient::msg(const string& recipient, const string& message)
{
    this->connection.send("PRIVMSG " + recipient + " :" + message);
}


void IrcClient::leave(const string& channel, const string& message)
{
    if (message.empty()) {
        this->connection.send("PART " + channel);
    } else {
        this->connection.send("PART " + channel + " :" + message);
    }
}

void IrcClient::whois(const string& nick)
{
    this->connection.send("WHOIS " + nick);
}
