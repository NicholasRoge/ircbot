#include <iostream>
#include <string>

#include "IrcConnection.h"


int main(int argc, char** argv)
{
    IrcConnection connection("irc.freenode.net");
    if (!connection.connect()) {
        std::cout << "[31mFailed to connect to server.[0m" << std::endl;

        return 1;
    }
    connection.setUser("BottyMcBotface", "Definitely not a bot.");
    connection.setNick("BottyMcBotface");
    connection.listen();
    connection.setNick("HerpMcDerp");
    connection.send("Keytap!~nicholasr@rrcs-24-227-248-174.sw.biz.rr.com", "WHAT?");
    connection.join("#laravel");
    connection.listen();
    connection.send("#laravel", "Do not be alarmed human.  This is just a test.");
    connection.listen();
    connection.leave("#laravel");
    connection.disconnect();

    return 0;
}
