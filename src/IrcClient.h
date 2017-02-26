#ifndef IRC_CLIENT_H
#define IRC_CLIENT_H

#include "IrcConnection.h"


using MessageCallbackTrigger = std::function<bool(const IrcMessage&)>;


MessageCallbackTrigger operator&&(const MessageCallbackTrigger& lop, const MessageCallbackTrigger& rop);

MessageCallbackTrigger operator||(const MessageCallbackTrigger& lop, const MessageCallbackTrigger& rop);

class IrcClient
{
public:
    using MessageCallback = std::function<void(IrcClient&, const IrcMessage&)>;


    IrcClient(const std::string& url, unsigned short port = 6667);

    ~IrcClient();


    bool connect(const std::string& user, const std::string& realName, int mode = 0);

    void disconnect(const std::string& message = "");

    bool connected() const;

    operator bool() const;


    void onMessage(const MessageCallback& callback);

    void onMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback);

    void onNextMessage(const MessageCallback& callback);

    void onNextMessage(const MessageCallbackTrigger& trigger, const MessageCallback& callback);

    void onCommand(const std::string& command, const MessageCallback& callback);

    void onCommand(const std::string& command, const MessageCallbackTrigger&, const MessageCallback& callback);

    void onNextCommand(const std::string& command, const MessageCallback& callback);

    void onNextCommand(const std::string& command, const MessageCallbackTrigger& trigger, const MessageCallback& callback);

    void onResponse(unsigned short code, const MessageCallback& callback);

    void onResponse(unsigned short code, const MessageCallbackTrigger& trigger, const MessageCallback& callback);

    void onNextResponse(unsigned short code, const MessageCallback& callback);

    void onNextResponse(unsigned short code, const MessageCallbackTrigger& trigger, const MessageCallback& callback);


    void waitFor(const MessageCallbackTrigger& trigger);

    void waitForCommand(const std::string& command);

    void waitForResponse(unsigned short code);


    void setNick(const std::string& nick, const std::string& password = "");

    void join(const std::string& channel);

    void msg(const std::string& recipient, const std::string& message);

    void leave(const std::string& channel, const std::string& message = "");

    void whois(const std::string& nick);

private:
    IrcConnection connection;

    std::string url;

    unsigned short port;

    std::string nick;

    std::string password;

    void authenticate(const std::string& user, const std::string& realName, int mode = 1);

    void setUser(const std::string& user, const std::string& realName, int mode = 0);
};

#endif
