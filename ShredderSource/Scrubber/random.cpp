#include "random.h"

#include <fstream>

using namespace boost;
using namespace std;

Random* Random::single = NULL;

/*
   Get instance
*/
Random* Random::GetInstance()
{
    if (single == NULL)
        single = new Random();

    return single;
}

/*
   Default constructer - Initializes random generator
*/
Random::Random()
{
    bool bSeedOK = false;
    unsigned int nSeed;

    ifstream fRand(PATH_URANDOM, ios::in);

    if (fRand.is_open())
    {
        fRand.read(reinterpret_cast<char*>(&nSeed), sizeof(nSeed));

        bSeedOK = !fRand.fail();

        fRand.close();
    }

    // If reading from urandom was ok, use it as seed, otherwise use default seed
    if (bSeedOK)
        mg.seed(nSeed);
}

/*
   Generates a random number between nMin and nMax
*/
int Random::GenRand(int nMin, int nMax)
{
    uniform_smallint<> dist(nMin, nMax);
    variate_generator<RNG&, uniform_smallint<> > gen(mg, dist);

    return gen();
}

/*
    Fills arrBuffer with nSize numbers ranging from nMin to nMax
*/
void Random::FillRand(char* arrBuffer, unsigned int nSize, int nMin, int nMax)
{
    uniform_smallint<> dist(nMin, nMax);
    variate_generator<RNG&, uniform_smallint<> > gen(mg, dist);

    for (unsigned int i = 0; i < nSize; i++)
    {
        arrBuffer[i] = gen();
    }
}

/*
    Same as fillrand - saves random generation overhead, to be tested
*/
void Random::FillRandInt(char* arrBuffer, unsigned int nSize, int nMin, int nMax)
{
    uniform_int<unsigned int> dist(nMin*nMin, nMax*nMax);
    variate_generator<RNG&, uniform_int<unsigned int> > gen(mg, dist);

    int* ptrInt = (int*)arrBuffer;

    for (unsigned int i = 0; i < nSize; i++)
    {
        ptrInt[i] = gen();
    }
}

/*

*/
void Random::FillRandString(char* arrBuffer, unsigned int nSize, const std::string* strPat)
{
    uniform_smallint<> dist(0, strPat->size() - 1);
    variate_generator<taus88&, uniform_smallint<> > gen(mg, dist);

    for (unsigned int i = 0; i < nSize; i++)
    {
        arrBuffer[i] = strPat->at(gen());
    }
}
