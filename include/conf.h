#pragma once
#include <map>
#include <Stream.h>

class ConfBlk: public std::map<String, String>
{
public:
    void dump(Stream& s);
    bool writeStream(Stream& s);
    bool writeFile();
    bool readStream(Stream& s);
    bool readFile();
};

extern const String conffilename;
extern ConfBlk configuration;