#include <util.h>


using std::string;


namespace util
{
    string to_hex(char b)
    {
        string str = "";

        char hi_nibble = (b & 0xF0) >> 4;
        if (hi_nibble < 0xA ) {
            hi_nibble += '0';
        } else {
            hi_nibble += 'A';
            hi_nibble -= 0xA;
        }
        str.append(&hi_nibble, 1);

        char low_nibble = b & 0x0F;
        if (low_nibble < 0xA ) {
            low_nibble += '0';
        } else {
            low_nibble += 'A';
            low_nibble -= 0xA;
        }
        str.append(&low_nibble, 1);
        
        return str;
    }


}
