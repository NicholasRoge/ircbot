#ifndef SOCKET_H
#define SOCKET_H

#include <functional>
#include <list>
#include <netdb.h>
#include <string>
#include <thread>


class Socket
{
public:
    using DataCallback = std::function<void(void*, size_t)>;


    Socket();

    ~Socket();

    bool connectTo(const std::string& url, unsigned short port);

    void disconnect();

    bool connected() const;

    void write(void* data, size_t byteCount);

    void write(const std::string& message);

    void onData(DataCallback callback);

    operator bool() const;

protected:
    addrinfo* getAddrInfo(const std::string& url, unsigned short port) const;

    bool tryConnectUsing(addrinfo* info);



private:
    int handle;

    addrinfo* target;

    std::thread listenThread;

    std::list<DataCallback> dataCallbacks;


    void startListenThread();

    void listen();
};

#endif
