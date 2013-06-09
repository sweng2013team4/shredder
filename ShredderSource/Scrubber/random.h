#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include <boost/random.hpp>

#define PATH_URANDOM "/dev/urandom"

#define RNG taus88

class Random
{
private:

    boost::RNG mg;

    static Random* single;

    Random();

public:
    static Random* GetInstance();

    int GenRand(int nMin, int nMax);
    void FillRand(char* arrBuffer, unsigned int nSize, int nMin, int nMax);
    void FillRandInt(char* arrBuffer, unsigned int nSize, int nMin, int nMax);
    void FillRandString(char* arrBuffer, unsigned int nSize,const std::string* strPat);
};

#endif // RANDOMIZER_H
