#include <log.h>

#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>

#include <util.h>


using std::asctime;
using std::fstream;
using std::localtime;
using std::strftime;
using std::string;
using std::time;
using std::to_string;
using util::to_hex;


namespace logger 
{
    pathstr dir = "/usr/local/var/log/ircbot"; 
    pathstr log = "session.log"; 
    string  timestamp_format = "%f %T%z";


    void write(message msg, pathstr out, std::string searchable)
    {
        string header = "";

        auto gmt = time(nullptr);
        auto local = localtime(&gmt);
        
        char buffer[1024];
        strftime(buffer, sizeof(buffer), timestamp_format.c_str(), local);
        header += "[" + string(buffer) + "]";

        header += " (" + to_string(msg.size()) + ")";

        if (!searchable.empty()) {
            header += " " + searchable;
        }


        system(("mkdir -p \"" + dir + "\"").c_str());


        std::fstream stream(dir + "/" + out, std::ios_base::binary | std::ios_base::app);
        stream << header << "\n";
        stream << msg    << "\n";

#ifdef DEBUG
        std::cout << header << std::endl;
        std::cout << msg    << std::endl;
#endif
    }

    void info(message msg, pathstr out, std::string searchable)
    {
        if (searchable.empty()) {
            searchable = "info";
        } else {
            searchable = "info " + searchable;
        }

        write(msg, out, searchable);
    }

    void error(message msg, pathstr out, std::string searchable)
    {
        if (searchable.empty()) {
            searchable = "error";
        } else {
            searchable = "error " + searchable;
        }

        write(msg, out, searchable);
    }

    void debug(message msg, pathstr out, std::string searchable)
    {
        if (searchable.empty()) {
            searchable = "debug";
        } else {
            searchable = "debug " + searchable;
        }


        write(msg, out, searchable);
    }

    void debug(const void* data, size_t data_size, pathstr out, std::string searchable)
    {
        message msg;


        auto iter = (char*)data;

        auto iter_end = iter + data_size;
        while (iter != iter_end) {
            string byte_view = "";
            string string_view = "";

            auto word_end = iter + 8;
            if (word_end > iter_end) {
                word_end = iter_end;
            }
            while (iter != word_end) {
                byte_view += to_hex(*iter);
                

                if (*iter > 0x20 && *iter < 0x80) {
                    string_view.append(iter, 1);
                } else {
                    string_view += ".";
                }


                if (iter < word_end) {
                    byte_view   += " ";
                    string_view += " ";
                }


                ++iter;
            }

            for (int i = byte_view.size(); i < 24; ++i) {
                byte_view += " ";
            }


            msg += byte_view.substr(0, 11) + "  " + byte_view.substr(12);
            msg += "  ";
            msg += string_view; 
            msg += "\n";
        }


        debug(msg, out, searchable);
    }
}
