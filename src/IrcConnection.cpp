#include "IrcConnection.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>


using std::string;


IrcConnection::IrcConnection(const string& server, unsigned short port)
{
    this->server = server;
    this->port = port;
    this->socketHandle = 0;
    this->serverInfo = nullptr;
}

IrcConnection::~IrcConnection()
{
    this->disconnect();
}

bool IrcConnection::connect()
{
    if (this->socketHandle) {
        return true;
    }

    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    char serverPort_whyIsThisAString[12];
    sprintf(serverPort_whyIsThisAString, "%d", this->port);
    int error1 = getaddrinfo(
        this->server.c_str(), 
        serverPort_whyIsThisAString,
        &hints, 
        &this->serverInfo
    );
    if (error1) {
        return false;
    }

    for (;this->serverInfo;this->serverInfo = this->serverInfo->ai_next) {
        this->socketHandle = socket(
            this->serverInfo->ai_family, 
            this->serverInfo->ai_socktype,
            this->serverInfo->ai_protocol
        );
        if (!this->socketHandle) {
            continue;
        }

        int error2 = ::connect(
            this->socketHandle, 
            this->serverInfo->ai_addr, 
            this->serverInfo->ai_addrlen
        );       
        if (error2 == -1) {
            close(this->socketHandle);
            this->socketHandle = 0;

            continue;
        }

        break;
    }
    if (!this->serverInfo) {
        return false;
    }

    std::cout << "Connected." << std::endl;
    return true;
}

void IrcConnection::disconnect()
{
    if (!this->socketHandle) {
        return;
    }

    close(this->socketHandle);
    this->socketHandle = 0;


    freeaddrinfo(this->serverInfo);
    this->serverInfo = nullptr;

    std::cout << "Disconnected." << std::endl;
}

void IrcConnection::command(const string& cmd, const string& args)
{
    string data = cmd + " " + args + "\r\n";
    std::cout << "[37m" << data << "[0m";
    sendto(this->socketHandle, data.c_str(), data.size(), 0, this->serverInfo->ai_addr, this->serverInfo->ai_addrlen);
    //TODO: Send through the socket
}

void IrcConnection::setUser(const string& user, const string& phrase)
{
    this->command("USER", user + " 2 * :" + phrase);
}

void IrcConnection::setNick(const string& nick)
{
    this->command("NICK", nick);
}

void IrcConnection::join(const string& channel)
{
    this->command("JOIN", channel);
}

void IrcConnection::send(const string& recipient, const string& message)
{
    this->command("PRIVMSG", recipient + " :" + message);
}


void IrcConnection::leave(const string& channel)
{
    this->command("PART", channel);
}

void IrcConnection::listen()
{
    char buffer[65536];

    pollfd  pfd;
    pfd.fd = this->socketHandle;
    pfd.events = POLLIN;
    while (poll(&pfd, 1, 30000)) {
        auto read = recvfrom(this->socketHandle, buffer, 65536, 0, this->serverInfo->ai_addr, &this->serverInfo->ai_addrlen);
        std::cout << "[32m" << string(buffer, read) << "[0m";
    }
}
