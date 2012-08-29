#ifndef CBUILDBLACKLIST_H
#define CBUILDBLACKLIST_H

#include <cstdio>

#include <vector>
using std::vector;

#include <string>
using std::string;

class CBuildBlackList
{
    private:
        FILE *fp;
        char File[1024];
        bool OpenFile();
        bool WriteContent(const char*);

    public:
        CBuildBlackList(const char*);
        ~CBuildBlackList();

        bool BuildBlackList(const vector<string>& bllist);
};

#endif /*CBUILDBLACKLIST_H*/
