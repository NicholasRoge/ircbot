#include <functional>
#include <list>
#include <string>
#include <vector>

#include "Socket.h"


struct IrcMessage
{
    std::string prefix;
    std::string command;
    std::vector<std::string> arguments;
    std::string tail;

    
    static IrcMessage parse(std::string s);

private:
    static std::string nextWord(std::string& s);
};

class IrcConnection
{
public:
    using MessageCallback = std::function<void(IrcConnection&, const IrcMessage&)>;


    IrcConnection(const std::string& server, unsigned short port = 6667);

    ~IrcConnection();

    bool connect(const std::string& nick, const std::string& user, const std::string& userComment = "");

    void disconnect();

    bool connected() const;

    operator bool() const;

    void command(const std::string& command, const std::string& args);

    void setUser(const std::string& user, const std::string& phrase);

    void setNick(const std::string& nick);

    void join(const std::string& channel);

    void send(const std::string& recipient, const std::string& message);

    void leave(const std::string& channel);

    void whois(const std::string& nick);

    void onMessage(MessageCallback callback);

private:
    Socket socket;

    std::string url;

    unsigned short port;

    std::string messagePartial;

    std::list<MessageCallback> messageCallbacks;


    void onData(void* data, size_t byteCount);
};
