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

#include <vector>
using std::vector;

struct blacklist
{
	string ip;
	string operation;
	string creator;

	blacklist() : ip(""),operation("REJECT"),creator("TAP") {}

};

class CDistribute
{
    public:
        CDistribute(string cms_ip,unsigned int cms_port);
        ~CDistribute();

        void setCmsStorePath(string path);
        void setDistributePath(string path);
        

        bool distribute(const CMachine& machine,const CDisFile& file);
		bool distributeBL(const string mem,const string sid,const string file);
        bool remoteExec(const CMachine& machine,const string& cmd);

        CDisFile getMaxVersionFile(int profile_id,int type);

        string getVersionByType(const CMachine& machine,const int type);
        string getTotalList();
        string getTotalNum();

		//Tap BlackList Process
		bool getTapBlackListFile(const CMachine& machine);
		bool parseTapBlackListFile(vector<blacklist>& blackLists);
		bool setTapBlackList(const string& host,const string& opt,const string& key,const string&sid);

    private:
        char* result;

        string cms_ip;
        unsigned int cms_port;

        string cms_store_path;
        string distribute_path;

		bool remoteCmdExec(const string& sid,const string& cmd);
        
};
#endif /*CDISTRIBUTE_H*/
