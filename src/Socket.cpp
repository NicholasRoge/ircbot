#include <Socket.h>

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <log.h>


using std::memset;
using std::runtime_error;
using std::string;
using std::thread;
using std::to_string;


Socket::Socket()
{
    this->handle = 0;
    this->listening = false;
}

Socket::~Socket()
{
    this->disconnect();
    if (this->listenThread.joinable()) {
        this->listenThread.join();
    }
}

bool Socket::connectTo(const string& url, unsigned short port = 6667) 
{
    if (this->handle) {
        this->disconnect();
    }

    auto info = this->getAddrInfo(url, port);
    while (info) {
        if (this->tryConnectUsing(info)) {
            this->target = info;

            break;
        }
        
        info = info->ai_next;
    }
    if (!this->target) {
        return false;
    }


    logger::debug("Socket " + to_string(this->handle) + " connected to " + url + ":" + to_string(port) + ".");


    this->startListenThread();

    return true;
}

addrinfo* Socket::getAddrInfo(const string& url, unsigned short port) const
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* info;
    int result = getaddrinfo(
        url.c_str(),
        to_string(port).c_str(),
        &hints,
        &info
    );
    if (result) {
        return nullptr;
    }

    return info;
}

bool Socket::tryConnectUsing(addrinfo* info)
{
    this->handle = socket(
        info->ai_family,
        info->ai_socktype,
        info->ai_protocol
    );
    if (!this->handle) {
        return false;
    }

    int result = connect(
        this->handle,
        info->ai_addr,
        info->ai_addrlen
    );
    if (result == -1) {
        close(this->handle);
        this->handle = 0;

        return false;
    }

    return true;
}

void Socket::disconnect()
{
    if (!this->handle) {
        return;
    }

    this->listening=false;
    if (this->listenThread.joinable()) {
        this->listenThread.join();
    }

    close(this->handle);
    this->handle = 0;

    freeaddrinfo(this->target);
    this->target = nullptr;
    

    logger::debug("Socket " + to_string(this->handle) + " disconnected.");


    for (auto callback : this->disconnectCallbacks) {
        callback(*this);
    }
}

bool Socket::connected() const
{
    return this->handle != 0;
}

void Socket::onDisconnect(DisconnectCallback callback)
{
    this->disconnectCallbacks.push_back(callback);
}

void Socket::write(void* data, size_t byteCount)
{
    if (!this->handle) {
        string error_message = "Attempted write to disconnected socket.";
        logger::error(error_message);
        throw runtime_error(error_message);
    }

    while (byteCount > 0) {
        auto bytesSent = sendto(
            this->handle,
            data,
            byteCount,
            0,
            this->target->ai_addr,
            this->target->ai_addrlen
        );

        if (bytesSent == -1) {
            string error_message = "Call to sendto failed (returned -1).  errno: " + to_string(errno);
            logger::error(error_message);
            throw runtime_error(error_message);
        }

#ifdef DEBUG
        logger::debug("Sent " + to_string(bytesSent) + " bytes on socket " + to_string(this->handle) + ".");
        logger::debug(data, bytesSent, "socket_" + to_string(this->handle) + "_out.log");
#endif

        data = (unsigned char*)data + bytesSent;
        byteCount -= bytesSent;
    }
}

void Socket::write(const string& message)
{
    this->write((void*)message.c_str(), message.size());
}

void Socket::onData(DataCallback callback)
{
    this->dataCallbacks.push_back(callback);
}

void Socket::startListenThread()
{
    if (this->listenThread.joinable()) {
        this->listenThread.join();
    }
    this->listenThread = thread([this]() {
        this->listen();
    });
}

void Socket::listen()
{
    char buffer[1024];

    pollfd pfd;
    pfd.fd = this->handle;
    pfd.events = POLLIN | POLLHUP;

    this->listening = true;
    while (this->listening) {
        auto result = poll(&pfd, 1, 0);
        if (result == -1) {
            string error_message = "Call to poll failed (returned -1).  errno:  " + to_string(errno);
            logger::error(error_message);
            throw runtime_error(error_message);
        } else if (result == 0) {
            std::this_thread::yield();

            continue;
        }

        if (pfd.revents & POLLHUP) {
            this->disconnect();
        } else {
            auto bytesRead = recv(this->handle, buffer, 1024, 0);
            if (bytesRead == -1) {
                if (errno == ENOTSOCK && !this->connected()) {
                    break;
                } else {
                    string error_message = "Call to recv failed (returned -1).  errno:  " + to_string(errno);
                    logger::error(error_message);
                    throw runtime_error(error_message);
                }
            } else {
#ifdef DEBUG
                logger::debug("Received " + to_string(bytesRead) + " bytes on socket " + to_string(this->handle) + ".");
                logger::debug(buffer, bytesRead, "socket_" + to_string(this->handle) + "_in.log");
#endif

                if (bytesRead == 0) {
                    this->disconnect();
                } else {
                    for (auto callback : this->dataCallbacks) {
                        callback(buffer, bytesRead);
                    }
                }
            }
        }
    }
}

Socket::operator bool() const
{
    return this->connected();
}
