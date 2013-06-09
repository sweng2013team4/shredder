#ifndef SCRUBPATTERN_H
#define SCRUBPATTERN_H

#include <vector>
#include <string>

#include "scrubpass.h"

class ScrubPattern
{
public:
    ScrubPattern(std::string sName);

    std::string sName;
    std::vector<ScrubPass> lstPasses;
};

#endif // SCRUBPATTERN_H
