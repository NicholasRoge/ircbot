#include <functional>
#include <list>
#include <string>
#include <vector>

#include <irc/message.h>
#include <Socket.h>


class IrcConnection
{
public:
    using MessageCallback = std::function<void(IrcConnection&, const irc::message&)>;

    using MessageFilter = std::function<bool(const irc::message&)>;
// TODO:  Rename to MessageRejector.
// TODO:  Define MessageSelector.

    IrcConnection();

    ~IrcConnection();

    bool connect(const std::string& url, unsigned short port);

    void disconnect();

    bool connected() const;

    operator bool() const;

    void send(const std::string& message);

    void send(const irc::message& message);

    void onMessage(MessageCallback callback, MessageFilter satisfying = nullptr);

    void onNextMessage(MessageCallback callback, MessageFilter satisfying = nullptr);

private:
    struct MessageCallbackObject
    {
        MessageCallback callback;
        MessageFilter   filter;
        bool            once;
    };


    Socket socket;

    std::string messagePartial;

    std::list<MessageCallbackObject> messageCallbacks;


    void onData(void* data, size_t byteCount);
};
