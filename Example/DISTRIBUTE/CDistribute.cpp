#include "CDistribute.h"
#include "Common.h"
#include <cstdio>

#include <iostream>
using std::cerr;
using std::endl;

#include <sstream>
using std::stringstream;

#include <fstream>
using std::fstream;

CDistribute::CDistribute(string cms_ip,unsigned int cms_port)
{
    this->cms_ip = cms_ip;
    this->cms_port = cms_port;
    this->cms_store_path = "";
    this->distribute_path = "";
    
    init_result(&(this->result));
}

CDistribute::~CDistribute()
{
    destroy_result(&(this->result));
}

void CDistribute::setCmsStorePath(string path)
{
    this->cms_store_path = path; 
}

void CDistribute::setDistributePath(string path)
{
    this->distribute_path = path;
}


bool CDistribute::distribute(const CMachine& machine,const CDisFile& file)
{
    string to_distribute = distribute_path + "/" + file.getFileName();
    stringstream ss;
    ss << cms_store_path;
    ss << "/";
    ss << file.getVersion();
    string to_cms = ss.str();

    if (false == my_copy(to_distribute.c_str(),to_cms.c_str()))
    {
        cerr << "copy file " << file.getFileName() << "error" << endl;
        return false;
    }

    stringstream cmd;
    cmd << "CopyFile\ntype:";
    cmd << file.getType() << "\n";
    cmd << "filename:";
    cmd << file.getVersion();

    stringstream filename;
    filename << file.getVersion();
    if (-1 == send_file(cms_ip.c_str(),cms_port,machine.getName().c_str(),cmd.str().c_str(),
                filename.str().c_str(),&(this->result)))
    {
        cerr << "send_file " << file.getFileName() << endl;
        unlink(to_cms.c_str());
        return false;
    }

    if (NULL == strstr(this->result,"\"error\":0"))
    {
        cerr << "send_file " << file.getFileName() << " Failed!"<< endl;
        unlink(to_cms.c_str());
        return false;
    }

    unlink(to_cms.c_str());
    return true;    
}

bool CDistribute::distributeBL(const string mem,const string sid,const string file)
{
    string to_distribute = distribute_path + "/" + file;
    string to_cms = cms_store_path + "/" + file;

    if (false == my_copy(to_distribute.c_str(),to_cms.c_str()))
    {
        cerr << "copy file " << file << "error" << endl;
        return false;
    }

    stringstream cmd;
    cmd << "TapmBllist\nfilename:";
    cmd << file;
	cmd << "\nmem:";
	cmd << mem;
	

    string zip_cmd = "cd " + cms_store_path;
    zip_cmd += "; tar zcf ";
    zip_cmd += file;
    zip_cmd += ".tgz ";
    zip_cmd += file;

    if (0 != system(zip_cmd.c_str()))
    {
        cerr << "shell:" << cmd << endl;
        unlink(to_cms.c_str());
        unlink((to_cms + ".tgz").c_str());
        unlink(to_distribute.c_str());
        
        return false;
    }

    if (-1 == send_file(cms_ip.c_str(),cms_port,sid.c_str(),cmd.str().c_str(),
                (file + ".tgz").c_str(),&(this->result)))
    {
        cerr << "send_file " << file << endl;
        unlink(to_cms.c_str());
        unlink((to_cms + ".tgz").c_str());
        unlink(to_distribute.c_str());
        return false;
    }

    if (NULL == strstr(this->result,"\"error\":0"))
    {
        cerr << "send_file " << file << " Failed!"<< endl;
        unlink(to_cms.c_str());
        unlink((to_cms + ".tgz").c_str());
        unlink(to_distribute.c_str());
        return false;
    }

    unlink(to_cms.c_str());
    unlink((to_cms + ".tgz").c_str());
    unlink(to_distribute.c_str());
    return true;    
}

bool CDistribute::remoteExec(const CMachine& machine,const string& cmd)
{
	return remoteCmdExec(machine.getName().c_str(),cmd);
}

string CDistribute::getVersionByType(const CMachine& machine,const int type)
{
    stringstream cmd;
    cmd << "GetVersion\ntype:";
    cmd << type;

    this->remoteExec(machine,cmd.str());

    return string(this->result);    
}

string CDistribute::getTotalList()
{
    list_ids(cms_ip.c_str(),cms_port,"*",&(this->result));
    return string(this->result);
}

string CDistribute::getTotalNum()
{
    list_ids(cms_ip.c_str(),cms_port,"",&(this->result));

    return string(this->result);
}

CDisFile CDistribute::getMaxVersionFile(int profile_id,int type)
{
    CDisFile tmp_;
    int max = -1;
    stringstream dir;
    dir << distribute_path;
    dir << "/";
    dir << profile_id;
    dir << "/";
    dir << type;

    struct dirent* ent = NULL;
    DIR *ptr = NULL;
    ptr = opendir(dir.str().c_str()); 

    if (ptr == NULL)
    {
        cerr << "can't open dir " << dir.str() << endl;
        return tmp_;
    }

    while ((ent = readdir(ptr)) != NULL)
    {
        if (ent->d_type == 8)
        {
            if (atoi(ent->d_name) > max)
            {
                max = atoi(ent->d_name);
            }
        }
    }

    closedir(ptr);
    ptr = NULL;

    if (max == -1)
    {
        return tmp_;
    }

    tmp_.setType(type); 
    tmp_.setVersion(max);
    tmp_.setProfileID(profile_id);

    return tmp_;
}

bool CDistribute::getTapBlackListFile(const CMachine& machine)
{
	if (false == remoteExec(machine,"MvBlackListFile"))
	{
		cerr << "MvBlackListFile Failed!" << endl;
		return false;
	}
	
	if (-1 == down_file( cms_ip.c_str(), cms_port, machine.getName().c_str(),
               "TAP_BLACKLIST_IP_TMP", &this->result))
	{
		cerr << "down_file TAP_BLACKLIST_IP_TMP Failed!" << endl;
		return false;
	}

	if (false == remoteExec(machine,"RmBlackListFile"))
	{
		cerr << "RmBlackListFile Failed!" << endl;
		return false;
	}

	return true;
}

bool CDistribute::parseTapBlackListFile(vector<blacklist>& blackLists)
{
	blackLists.clear();

	fstream fp((cms_store_path+"/TAP_BLACKLIST_IP_TMP").c_str());
	string line_buffer;

	while(getline(fp,line_buffer))
	{
		blacklist buffer;
		string::size_type pos1 = line_buffer.find(" ");
		if (pos1 != string::npos)
		{
			buffer.ip = line_buffer.substr(0,pos1-0);
			string::size_type pos2 = line_buffer.find(" ",++pos1);
			if (pos2 != string::npos)
			{
				buffer.operation = line_buffer.substr(pos1,pos2-pos1);
				buffer.creator = line_buffer.substr(++pos2,string::npos);
			}

			blackLists.push_back(buffer);
		}
		else if (line_buffer.size() >= 7)
		{
			buffer.ip = replace_all(line_buffer,"\r","");
			blackLists.push_back(buffer);
		}
	}

	unlink((cms_store_path+"/TAP_BLACKLIST_IP_TMP").c_str());

	return true;
}

bool CDistribute::setTapBlackList(const string& host,const string& opt,const string& key,const string& sid)
{
	string cmd = "BlackListOpt\nhost:" + host + "\nopt:" + opt + "\nkey:" + key;

	return remoteCmdExec(sid,cmd);
}

bool CDistribute::remoteCmdExec(const string& sid,const string& cmd)
{
    if (-1 == send_ids(cms_ip.c_str(),cms_port,sid.c_str(),
            cmd.c_str(),&(this->result)))
    {
        cerr << "send_ids exec " << cmd << " Failed" << endl; 
        return false;
    }
 
    if (NULL == strstr(this->result,"\"error\":0"))
    {
        cerr << "send_ids exec " << cmd << " Failed" << endl; 
        return false;
    }

	return true;
}
