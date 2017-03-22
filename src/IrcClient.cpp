#include <IrcClient.h>

#include <iostream>
#include <cstring>
#include <thread>

#include <Base64.h>
#include <InterruptedException.h>
#include <log.h>


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
    return [lop, rop](const irc::message& message) {
        return lop(message) && rop(message);
    };
}

MessageCallbackTrigger operator||(const MessageCallbackTrigger& lop, const MessageCallbackTrigger& rop)
{
    return [lop, rop](const irc::message& message) {
        return lop(message) || rop(message);
    };
}

IrcClient::IrcClient(const std::string& url, unsigned short port)
{
    this->url = url;
    this->port = port;

    this->onCommand("PING", [](IrcClient& server, const irc::message& message) {
        server.connection.send("PONG :" + message.tail());
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

    this->authenticate(user, realName, mode);

    this->waitForResponse(1);

    return true;
}

void IrcClient::authenticate(const std::string& user, const std::string& realName, int mode)
{
    if (!this->password.empty()) {
        this->connection.send("CAP REQ :sasl");
        this->onNextMessage(
            [](const irc::message& message) {
                string command = message.command();
                if (command != "CAP") {
                    return false;
                }

                if (message.arg(1) != "ACK" && message.arg(1) != "NACK") {
                    return false;
                }
                
                string tail = message.tail();
                if (tail.find("sasl") == string::npos) {
                    return false;
                }

                return true;
            },
            [user](IrcClient& client, const irc::message& message) {
                if (message.arg(1) == "ACK") {
                    client.connection.send("AUTHENTICATE PLAIN");
                    client.onNextMessage(
                        [](const irc::message& message) {
                            auto arg = message.arg(0);
                            logger::debug(arg.c_str(), arg.length());
                            return message.command() == "AUTHENTICATE" && message.arg(0) == "+";
                        },
                        [user](IrcClient& client, const irc::message& message) {
                            string authDetails = "";
                            authDetails.append(user);
                            authDetails.append("\0", 1);
                            authDetails.append(client.nick);
                            authDetails.append("\0", 1);
                            authDetails.append(client.password);
                            authDetails = Base64Encode((unsigned const char*)authDetails.c_str(), authDetails.length());

                            client.connection.send("AUTHENTICATE " + authDetails);
                            client.onNextMessage(
                                [](const irc::message& message) {
                                    return message.command() == "900" || message.command() == "904";
                                },
                                [](IrcClient& client, const irc::message& message) {
                                    client.connection.send("CAP END");
                                    // TODO:  trigger authenticated true/false event
                                }
                            );
                        }
                    );
                } else {
                    // TODO:  Fallback authentication method
                }
            }
        );
        // TODO:  Find a better way to do these asynchronous transactions
        // TODO:  Oh dear god find a way.
    }

    this->setUser(user, realName, mode);
    if (this->nick.empty()) {
        this->setNick(user, "");
    } else {
        this->setNick(this->nick, this->password);
    }
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
        [this, callback](IrcConnection& connection, const irc::message& message) {
            callback(*this, message);
        }
    );
}

void IrcClient::onMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    // Temporary until I get around to reworking the IrcConnection interface
    // as well
    this->connection.onMessage(
        [this, callback](IrcConnection& connection, const irc::message& message) {
            callback(*this, message);
        },
        [trigger](const irc::message& message) {
            return !trigger(message);
        }
   );
}

void IrcClient::onNextMessage(const MessageCallback& callback)
{
    this->connection.onNextMessage(
        [this, callback](IrcConnection& connection, const irc::message& message) {
            callback(*this, message);
        }
    );
}

void IrcClient::onNextMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    // Temporary until I get around to reworking the IrcConnection interface
    // as well
    this->connection.onNextMessage(
        [this, callback](IrcConnection& connection, const irc::message& message) {
            callback(*this, message);
        },
        [trigger](const irc::message& message) {
            return !trigger(message);
        }
   );
};

void IrcClient::onCommand(const std::string& command, const MessageCallback& callback)
{
    this->onMessage(
        [command](const irc::message& message) {
            return message.command() == command;
        },
        callback
    );
}

void IrcClient::onCommand(const std::string& command, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onMessage(
        [command](const irc::message& message) {
            return message.command() == command;
        } && trigger,
        callback
    );
}

void IrcClient::onNextCommand(const std::string& command, const MessageCallback& callback)
{
    this->onNextMessage(
        [command](const irc::message& message) {
            return message.command() == command;
        },
        callback
    );
}

void IrcClient::onNextCommand(const std::string& command, const MessageCallbackTrigger& trigger, const MessageCallback& callback)
{
    this->onNextMessage(
        [command](const irc::message& message) {
            return message.command() == command;
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
        [&block](IrcClient& server, const irc::message& message) {
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
    this->waitFor([command](const irc::message& message) {
        return message.command() == command;
    });
}

void IrcClient::waitForResponse(unsigned short code)
{
    this->waitForCommand(ResponseToCommand(code));
}

void IrcClient::setUser(const string& user, const string& realName, int mode)
{
    irc::message message;
    message.command("USER");
    message.arg(user);
    message.arg(to_string(mode));
    message.arg("*");
    message.tail(realName);
    this->connection.send(message);
}

void IrcClient::setNick(const string& nick, const string& password)
{
    this->nick = nick;
    if (this->connection) {
        this->connection.send("NICK " + nick);
    }

    this->password = password;
    if (this->connection) {
        if (!password.empty()) {
            // TODO:  Authenticate the user after a nick change.  Can I reuse
            // the IrcClient#authenticate method somehow...?
        }
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
