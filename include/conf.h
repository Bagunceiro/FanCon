#pragma once
#include <map>
#include <Stream.h>

class ConfBlk: public std::map<String, String>
{
public:
    void dump();
    bool writeFile(Stream& s);
    bool writeFile();
    bool readFile(Stream& s);
    bool readFile();
};

extern const String conffilename;
extern ConfBlk configuration;