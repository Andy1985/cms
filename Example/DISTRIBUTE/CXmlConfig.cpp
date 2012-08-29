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
    if (NULL == log_file) return false;
    config.log_file = log_file->GetText();

	TiXmlElement *bllist_serv = log_file->NextSiblingElement();
	if (NULL == bllist_serv) return false;

	TiXmlElement *_item = bllist_serv->FirstChildElement();
	while (_item != NULL)
	{
		profile_sid buffer;

		TiXmlElement *_profile_id = _item->FirstChildElement();
		if (NULL == _profile_id) return false;
		buffer.profile_id = my_atoi(_profile_id->GetText());
		
		TiXmlElement *_mems = _profile_id->NextSiblingElement();
		if (NULL == _mems) return false;

		TiXmlElement *_psm = _mems->FirstChildElement();
		while (_psm != NULL)
		{
			psm buf;
			TiXmlElement *_sid = _psm->FirstChildElement();
			if (NULL == _sid) return false;
			buf.sid = string(_sid->GetText());

			TiXmlElement *_mem = _sid->NextSiblingElement();
			if (NULL == _mem) return false;
			buf.mem = string(_mem->GetText());

			_psm = _psm->NextSiblingElement();

			buffer.psms.push_back(buf);
		}
		
		config.psids.push_back(buffer);

		_item = _item->NextSiblingElement();
	}

    return true;
}
