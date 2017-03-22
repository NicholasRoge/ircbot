#include <array>
#include <string>
#include <vector>


namespace irc
{
    class message
    {
    public:
        message();

        message(const std::string& s);

        std::string prefix() const;

        void prefix(const std::string& prefix);

        std::string origin() const;

        void origin(const std::string& origin);

        std::string server() const;

        void server(const std::string& prefix);

        std::string nick() const;

        void nick(const std::string& nick);

        std::string user() const;

        void user(const std::string& user);

        std::string host() const;

        void host(const std::string& host);

        std::string command() const;

        void command(const std::string& command);

        std::vector<std::string> args() const;

        void args(std::string args);

        void args(words args);

        void args(words::iterator first, words::iterator last);

        void arg(size_t offset, const std::string& arg);
        
        std::string tail() const;

        void tail(const std::string& tail);

        operator std::string() const;

    private:
        std::string origin;

        std::string user;

        std::string host;

        std::string command;
        
        words       arguments;

        std::string tail
    };
}
