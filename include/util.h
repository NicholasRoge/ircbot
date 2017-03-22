#ifndef UTIL_H
#define UTIL_H

#include <string>


namespace util
{
    template <typename type>
    std::string to_hex(type);

    std::string to_hex(char);

    int from_hex(std::string);


    template <typename type>
    std::string to_hex(type o)
    {
        std::string str = "";
        
        for (size_t i = 0 ; i < sizeof(type) ; ++i) {
            str += to_hex(((char*)&o)[i]);
        }

        return str;
    }
}


#endif
