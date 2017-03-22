#include <array>
#include <exception>
#include <functional>
#include <string>
#include <list>


namespace irc
{
    using word     = std::string;
    using sentence = std::list<word>;
    using offset   = size_t;

    using listener = std::function<void(word&)>;
    using speaker  = std::function<sentence()>;

    using invalid_sentence_exception = std::runtime_error;


    class message
    {
    public:
        message();

        message(sentence);

        explicit message(std::string);


        static sentence parse(std::string);


        word     prefix() const;

        message& prefix(word);


        word     server() const;

        message& server(word);


        word     nick() const;

        message& nick(word);


        word     user() const;

        message& user(word);


        word     host() const;

        message& host(word);


        word     command() const;

        message& command(word);


        message& arg(word);

        message& arg(offset, word);

        word     arg(offset) const;

        message& arg(listener);
        

        sentence args() const;

        message& args(sentence);

        message& args(std::string);

        message& args(sentence::iterator, sentence::iterator);

        message& args(speaker);


        word tail() const;

        message& tail(word);


        size_t size() const;


        explicit operator std::string() const;


    private:
        word     _prefix;
        word     _command;
        sentence _args;
        word     _tail;
    };
}
