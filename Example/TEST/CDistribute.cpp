#include "CDistribute.h"
#include "Common.h"
#include <cstdio>

#include <iostream>
using std::cerr;
using std::endl;

#include <sstream>
using std::stringstream;

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

bool CDistribute::remoteExec(const CMachine& machine,const string& cmd)
{
    if (-1 == send_ids(cms_ip.c_str(),cms_port,machine.getName().c_str(),
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
