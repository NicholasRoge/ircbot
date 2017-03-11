#include <IrcMessage.h>


using std::string;
using std::vector;


string ExtractNextWord(string& s);


string ExtractNextWord(string& s)
{
    auto end = s.find(" ");
    if (end == string::npos) {
        string word(s);
        s.clear();
        return word;
    } else {
        string word(s.begin(), s.begin() + end);
        s.erase(0, end + 1);
        return word;
    }
}

IrcMessage::IrcMessage(string s)
{
    auto crlfPos = s.find("\r\n");
    if (crlfPos != string::npos) {
        s.erase(crlfPos);
    }

    if (s[0] == ':') {
        s.erase(0, 1);
        this->setPrefix(ExtractNextWord(s));
    }
    this->setCommand(ExtractNextWord(s));
    this->setArgs(s);
}

string IrcMessage::getPrefix() const
{
    if (this->hasServer()) {
        return this->getServer();
    } else if (this->hasNick()) {
        string prefix = this->getNick();

        if (this->hasHost()) {
            if(this->hasUser()) {
                prefix += "!" + this->getUser();
            }

            prefix += "@" + this->getHost();
        }

        return prefix;
    }

    return "";
}

void IrcMessage::setPrefix(const string& prefix)
{
    auto atPos = prefix.find("@");
    if (atPos == string::npos) {
        this->setOrigin(prefix);

        return;
    }

    this->setHost(prefix.substr(atPos + 1));

    auto bangPos = prefix.find("!");
    if (bangPos == string::npos) {
        this->setOrigin(prefix.substr(0, atPos));

        return;
    }

    this->setOrigin(prefix.substr(0, bangPos));
    this->setUser(prefix.substr(bangPos + 1, atPos - (bangPos + 1)));
}

bool IrcMessage::hasPrefix() const
{
    return !this->origin.empty();
}

string IrcMessage::getOrigin() const
{
    return this->origin;
}

void IrcMessage::setOrigin(const string& origin)
{
    this->origin = origin;
    if (origin == "") {
        this->setUser("");
        this->setHost("");
    }
}

bool IrcMessage::hasOrigin() const
{
    return !this->origin.empty();
}

string IrcMessage::getServer() const
{
    return this->getOrigin();
}

void IrcMessage::setServer(const string& server)
{
    this->setOrigin(server);
    this->setUser("");
    this->setHost("");
}

bool IrcMessage::hasServer() const
{
    return this->hasOrigin();
}

string IrcMessage::getNick() const
{
    return this->getOrigin();
}

void IrcMessage::setNick(const string& nick)
{
    this->setOrigin(nick);
}

bool IrcMessage::hasNick() const
{
    return this->hasOrigin();
}

string IrcMessage::getUser() const
{
    return this->user;
}

void IrcMessage::setUser(const string& user)
{
    this->user = user;
}

bool IrcMessage::hasUser() const
{
    return !this->user.empty();
}

string IrcMessage::getHost() const
{
    return this->host;
}

void IrcMessage::setHost(const string& host)
{
    this->host = host;
}

bool IrcMessage::hasHost() const
{
    return !this->host.empty();
}

string IrcMessage::getCommand() const
{
    return this->command;
}

void IrcMessage::setCommand(const string& command)
{
    this->command = command;
}

bool IrcMessage::hasCommand() const
{
    return !this->command.empty();
}

vector<string> IrcMessage::getArgs() const
{
    return this->args;
}

void IrcMessage::setArgs(string args)
{
    this->args.clear();

    while (!args.empty()) {
        if (args[0] == ':') {
            args.erase(0, 1);
            this->setTrailing(args);
            break;
        } else {
            this->args.push_back(ExtractNextWord(args));
        }
    }
}

void IrcMessage::setArgs(vector<string> args)
{
    this->args = args;
}

bool IrcMessage::hasArgs() const
{
    // Technically, I think this should be
    // `return !this->args.empty() || this->hasTrailing();` but for the sake of
    // not confusing myself later when hasArgs is returning true but getArgs
    // returns a vector of size zero...
    return !this->args.empty();
}

size_t IrcMessage::getArgCount() const
{
    // Again, I think this should technically take hasTrailing into account, 
    // but...  ^
    return this->args.size();
}

string IrcMessage::getArg(size_t offset) const
{
    return this->args[offset];
}

void IrcMessage::setArg(size_t offset, const string& arg)
{
    this->args[offset] = arg;
}

void IrcMessage::insertArg(size_t offset, const string& arg)
{
    this->args.insert(this->args.begin() + offset, arg);
}

void IrcMessage::prependArg(const string& arg)
{
    this->insertArg(0, arg);
}

void IrcMessage::appendArg(const string& arg)
{
    this->args.push_back(arg);
}

void IrcMessage::removeArg(size_t offset)
{
    this->args.erase(this->args.begin() + offset);
}

string IrcMessage::getTrailing() const
{
    return this->trailing;
}

void IrcMessage::setTrailing(const string& trailing)
{
    this->trailing = trailing;
}

bool IrcMessage::hasTrailing() const
{
    return !this->trailing.empty();
}

string IrcMessage::toString() const
{
    string strRep = "";
    
    if (this->hasPrefix()) {
        strRep += ":" + this->getPrefix();
    }
    
    strRep += " " + this->getCommand();
    
    for (auto& arg : this->getArgs()) {
        strRep += " " + arg;
    }

    if (this->hasTrailing()) {
        strRep += " :" + this->getTrailing();
    }
    
    strRep += "\r\n";

    return strRep;
}

bool IrcMessage::isResponse() const
{
    if (this->command.length() != 3) {
        return false;
    }

    for (auto& c : this->command) {
        if (c < '0' || c > '9') {
            return false;
        }
    }

    return true;
}
