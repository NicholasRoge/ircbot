#include <netdb.h>
#include <string>


class IrcConnection
{
public:
    IrcConnection(const std::string& server, unsigned short port = 6667);

    ~IrcConnection();

    bool connect();

    void disconnect();

    void command(const std::string& command, const std::string& args);

    void setUser(const std::string& user, const std::string& phrase);

    void setNick(const std::string& nick);

    void join(const std::string& channel);

    void send(const std::string& recipient, const std::string& message);

    void leave(const std::string& channel);

    void listen();

private:
    std::string server;

    unsigned short port;

    int socketHandle;

    addrinfo* serverInfo;
};
