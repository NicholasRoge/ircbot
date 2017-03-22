#include <irc/message.h>

#include <iostream>
#include <regex>

#include <log.h>
#include <util.h>


using std::advance;
using std::regex;
using std::regex_error;
using std::regex_search;
using std::string;
using std::to_string;
using util::to_hex;


namespace irc 
{
    message::message()
    {
    }

    message::message(sentence msg)
    {
        if (msg.empty()) {
            throw invalid_sentence_exception("empty");
        }

        if (msg.front()[0] == ':') {
            if (msg.size() == 1) {
                throw invalid_sentence_exception("missing command");
            }
        }


        if (msg.front()[0] == ':') {
            this->prefix(msg.front());
            msg.pop_front();
        }

        this->command(msg.front());
        msg.pop_front();

        if (!msg.empty()) {
            if (msg.back()[0] == ':') {
                this->tail(msg.back());
                msg.pop_back();
            }

            this->args(msg);
        }
    }

    message::message(string str)
    : message(message::parse(str))
    {
    }

    word message::prefix() const
    {
        if (!this->server().empty()) {
            return this->server();
        }
        
        if (!this->nick().empty()) {
            string prefix = this->nick();

            if (!this->host().empty()) {
                if(!this->user().empty()) {
                    prefix += "!" + this->user();
                }

                prefix += "@" + this->host();
            }

            return prefix;
        }

        return "";
    }


    sentence message::parse(string str)
    {
        auto crlf_pos = str.find("\r\n");
        if (crlf_pos != string::npos) {
            str.erase(crlf_pos);
        }


        word tail;

        auto tail_pos = str.find(" :");
        if (tail_pos != string::npos) {
            tail = str.substr(tail_pos + 1);
            str.erase(tail_pos);
        } else {
            tail = ":";
        }


        sentence result;

        while (!str.empty()) {
            auto space_pos = str.find(" ");
            if (space_pos == string::npos) {
                result.push_back(str);
                
                str = "";
            } else {
                auto w = str.substr(0, space_pos);
                
                result.push_back(w);

                str.erase(0, space_pos + 1);
            }
        }
        

        if (!tail.empty()) {
            result.push_back(tail);
        }


        return result;
    }


    message& message::prefix(word prefix)
    {
        // Trim whitespace
        prefix.erase(0, prefix.find_first_not_of(" "));
        prefix.erase(prefix.find_last_not_of(" "));


        /*static const string VALID_WORD = "[\x01-\x09\x0B-\x0C\x0E-\x1F\x21-\x39\x3B-\xFF]+";
        static const regex PREFIX_REGEX("^:?" + VALID_WORD + "(?:@(?:" + VALID_WORD + "!)?" + VALID_WORD + ")?(?:\r\n)?$");
  
        if (!regex_search(prefix.begin(), prefix.end(), PREFIX_REGEX)) {
            throw invalid_sentence_exception("invalid prefix");
        }*/

        
        if (prefix[0] == ':') {
            prefix.erase(0, 1);
        }
        this->_prefix = prefix;


        return *this;
    }


    string message::server() const
    {
        return this->_prefix;
    }

    message& message::server(word server)
    {
        this->prefix(server);

        
        return *this;
    }


    string message::nick() const
    {
        auto bang_pos = this->_prefix.find("!");
        if (bang_pos != string::npos) {
            return this->_prefix.substr(0, bang_pos);
        }

        auto at_pos = this->_prefix.find("@");
        if (at_pos != string::npos) {
            return this->_prefix.substr(0, at_pos);
        }

        return this->_prefix;
    }


    message& message::nick(word nick)
    {
        auto bang_pos = this->_prefix.find("!");
        if (bang_pos != string::npos) {
            this->_prefix.erase(0, bang_pos);
            this->_prefix.insert(0, nick);
        }

        auto at_pos = this->_prefix.find("@");
        if (at_pos != string::npos) {
            this->_prefix.erase(0, at_pos);
            this->_prefix.insert(0, nick);
        }

        this->_prefix = nick;


        return *this;
    }


    string message::user() const
    {
        auto bang_pos = this->_prefix.find("!");
        if (bang_pos == string::npos) {
            return "";
        }

        auto at_pos = this->_prefix.find("@");
        if (at_pos == string::npos) {
            this->_prefix.substr(bang_pos + 1);
        } else {
            this->_prefix.substr(bang_pos + 1, at_pos - bang_pos - 1);
        }

        return "";
    }

    message& message::user(word w)
    {
        auto bang_pos = this->_prefix.find("!");
        auto at_pos   = this->_prefix.find("@");

        if (bang_pos == string::npos) {
            this->_prefix.insert(at_pos, "!" + w);
        } else {
            this->_prefix.erase(bang_pos + 1, at_pos);
            this->_prefix.insert(at_pos, w);
        }


        return *this;
    }


    string message::host() const
    {
        auto at_pos = this->_prefix.find("@");
        if (at_pos == string::npos) {
            return "";
        } else {
            return this->_prefix.substr(at_pos + 1);
        }
    }

    message& message::host(word w)
    {
        auto at_pos = this->_prefix.find("@");
        if (at_pos != string::npos) {
            this->_prefix.erase(at_pos);
        }
        
        this->_prefix += "@" + w;
    }


    string message::command() const
    {
        return this->_command;
    }

    message& message::command(word w)
    {
        this->_command = w;


        return *this;
    }


    word message::arg(offset o) const
    {
        if (o >= this->_args.size()) {
            return "";
        }


        auto iter = this->_args.begin();

        advance(iter, o);

        return *iter;
    }

    message& message::arg(word w)
    {
        this->_args.push_back(w);


        return *this;
    }

    message& message::arg(offset o, word w)
    {
        auto iter = this->_args.begin();

        advance(iter, o);

        *iter = w;


        return *this;
    }

    message& message::arg(listener l)
    {
        for (auto& arg : this->_args) {
            l(arg);
        }


        return *this;
    }


    sentence message::args() const
    {
        return this->_args;
    }

    message& message::args(string args)
    {
        this->args(message::parse(args));


        return *this;
    }


    message& message::args(sentence args)
    {
        this->args(args.begin(), args.end());


        return *this;
    }

    message& message::args(sentence::iterator first, sentence::iterator last)
    {
        this->_args.clear();
        this->_args.insert(this->_args.begin(), first, last);


        return *this;
    }


    string message::tail() const
    {
        return this->_tail;
    }

    message& message::tail(word w)
    {
        if (!w.empty() && w[0] == ':') {
            w.erase(0, 1);
        }

        this->_tail = w;


        return *this;
    }

    size_t message::size() const
    {
        return string(*this).size();
    }

    message::operator string() const
    {
        string result = "";
        
        if (!this->prefix().empty()) {
            result += ":" + this->prefix() + " ";
        }
        
        result += this->command();
        
        for (auto& arg : this->args()) {
            result += " " + arg;
        }

        if (!this->tail().empty()) {
            result += " :" + this->tail();
        }
        
        result += "\r\n";

        return result;
    }
}
