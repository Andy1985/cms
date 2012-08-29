#ifndef _CXMLCONFIG_H_
#define _CXMLCONFIG_H_

#include "tinyxml.h"
#include <string>
using std::string;

#include <vector>
using std::vector;

struct Config
{
    string oracle_user;
    string oracle_pass;
    string oracle_sid;

    string cms_ip;
    unsigned int cms_port;
    string cms_file_storge_path;
    
    string distribute_file_path;

    string log_file;

    Config()
    {
        oracle_user = "";
        oracle_pass = "";
        oracle_sid = "";

        cms_ip = "";
        cms_port = 0;
        cms_file_storge_path = "";
        
        distribute_file_path = "";
        log_file = "";
    }
    
};

class CXmlConfig
{
  public:
    CXmlConfig (string config);
    ~CXmlConfig ();
    bool GetConfig(struct Config &config);
  private:
    TiXmlDocument * myDoc;
    string cfile;
};

#endif /*_CXMLCONFIG_H_*/
