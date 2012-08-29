#include "Common.h"
#include "CXmlConfig.h"
#include <string.h>

#include <iostream>
using std::cerr;
using std::endl;

CXmlConfig::CXmlConfig (string config)
{
    myDoc = new TiXmlDocument ();
    cfile = config;
}

CXmlConfig::~CXmlConfig ()
{
    delete myDoc;
}

bool CXmlConfig::GetConfig (struct Config &config)
{
    if (false == myDoc->LoadFile (cfile.c_str()))
    {
        cerr << "load " << cfile << " file failed!" << endl;
        return false;
    }

    TiXmlElement *root = myDoc->RootElement ();

    if (NULL == root) return false;

    TiXmlElement *oracle = root->FirstChildElement ();
    if (NULL == oracle) return false;

    TiXmlElement *user = oracle->FirstChildElement ();
    if (NULL == user) return false;
    config.oracle_user = user->GetText();

    TiXmlElement *pass = user->NextSiblingElement();
    if (NULL == pass) return false;
    config.oracle_pass = pass->GetText();

    TiXmlElement *SID = pass->NextSiblingElement ();
    if (NULL == SID) return false;
    config.oracle_sid = SID->GetText();

    TiXmlElement *CMS = oracle->NextSiblingElement();
    if (NULL == CMS) return false;

    TiXmlElement *cms_ip = CMS->FirstChildElement();
    if (NULL == cms_ip) return false;
    config.cms_ip = cms_ip->GetText();

    TiXmlElement *cms_port = cms_ip->NextSiblingElement();
    if (NULL == cms_port) return false;
    config.cms_port = my_atoi(cms_port->GetText());    

    TiXmlElement *cms_file_storge_path = cms_port->NextSiblingElement();
    if (NULL == cms_file_storge_path) return false;
    config.cms_file_storge_path = cms_file_storge_path->GetText();

    TiXmlElement *distribute_file_path = CMS->NextSiblingElement();
    if (NULL == distribute_file_path) return false;
    config.distribute_file_path = distribute_file_path->GetText();

    TiXmlElement *log_file = distribute_file_path->NextSiblingElement();
    if (false == log_file) return false;
    config.log_file = log_file->GetText();

    return true;
}
