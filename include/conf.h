#include <map>

class ConfBlk: public std::map<String, String>
{
public:
    void dump();
    bool writeFile();
    bool readFile();
} confBlk;

extern const String conffilename;