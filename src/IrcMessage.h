#include <string>
#include <vector>


class IrcMessage
{
public:
    IrcMessage(std::string s = "");

    std::string getPrefix() const;

    void setPrefix(const std::string& prefix);

    bool hasPrefix() const;

    std::string getOrigin() const;

    void setOrigin(const std::string& origin);

    bool hasOrigin() const;

    std::string getServer() const;

    void setServer(const std::string& prefix);

    bool hasServer() const;

    std::string getNick() const;

    void setNick(const std::string& nick);

    bool hasNick() const;

    std::string getUser() const;

    void setUser(const std::string& user);

    bool hasUser() const;

    std::string getHost() const;

    void setHost(const std::string& host);

    bool hasHost() const;

    std::string getCommand() const;

    void setCommand(const std::string& command);

    bool hasCommand() const;

    std::vector<std::string> getArgs() const;

    void setArgs(std::string args);

    void setArgs(const std::vector<std::string>& args);

    bool hasArgs() const;

    size_t getArgCount() const;

    std::string getArg(size_t offset) const;

    void setArg(size_t offset, const std::string& arg);

    void insertArg(size_t offset, const std::string& arg);

    void prependArg(const std::string& arg);

    void appendArg(const std::string& arg);

    void removeArg(size_t offset);

    std::string getTrailing() const;

    void setTrailing(const std::string& trailing);

    bool hasTrailing() const;

    std::string toString() const;

    bool isResponse() const;

private:
    std::string origin;

    std::string user;

    std::string host;

    std::string command;
    
    std::vector<std::string> args;
    
    std::string trailing;
};
