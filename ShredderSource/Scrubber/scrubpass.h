#ifndef SCRUBPASS_H
#define SCRUBPASS_H

#include <vector>

class ScrubPass
{
public:

    // Enum
    enum PASS_TYPE
    {
        NORMAL,
        VERIFY,
        RANDOM
    };

    PASS_TYPE pType;
    std::vector<char> arr;

    ScrubPass();
};

#endif // SCRUBPASS_H
