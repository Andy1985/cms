#ifndef CDISTRIBUTE_H
#define CDISTRIBUTE_H

#include "CMachine.h"
#include "CDisFile.h"
#include "Communication.h"

#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>

#include <string>
using std::string;

class CDistribute
{
    public:
        CDistribute(string cms_ip,unsigned int cms_port);
        ~CDistribute();

        void setCmsStorePath(string path);
        void setDistributePath(string path);
        

        bool distribute(const CMachine& machine,const CDisFile& file);
        bool remoteExec(const CMachine& machine,const string& cmd);

        CDisFile getMaxVersionFile(int profile_id,int type);

        string getVersionByType(const CMachine& machine,const int type);
        string getTotalList();
        string getTotalNum();

    private:
        char* result;

        string cms_ip;
        unsigned int cms_port;

        string cms_store_path;
        string distribute_path;

        
};
#endif /*CDISTRIBUTE_H*/
