////////////////////////////////////////////////////////////////////////////////////////////////
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>
using std::string;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::stringstream;

#include "CXmlConfig.h"
#include "CDBOption.h"
#include "CDisFile.h"
#include "CMachine.h"
#include "CDistribute.h"
#include "CRunLog.h"
#include "CBuildBlackList.h"
#include "Common.h"

const int BLACKLIST_TYPE = 5;

////////////////////////////////////////////////////////////////////////////////////////////////
struct _global_config
{
    int do_all;
    int do_daemon;
    int profile_id;
    int machine_id;
    int conf_file_type;
    int conf_file_version;
    int do_list;
	int do_blacklist;
	int do_recover;

    string config;
	string key;
	string opt;

    _global_config()
    {
        do_all = -1;
        do_daemon = -1;
        profile_id = -1;
        machine_id = -1;
        conf_file_type = -1;
        conf_file_version = -1;
        do_list = -1;
		do_blacklist = -1;
		do_recover = -1;

        config = "";
		key = "";
		opt = "";
    }

} global_config;
static const char *opt_string = "LDABRo:k:f:p:m:t:v:h?";
static const struct option long_opts[] = {
    { "daemon", no_argument, NULL, 'D' },
    { "all", no_argument, NULL, 'A' },
    { "list", no_argument, NULL, 'L' },
    { "blacklist", no_argument, NULL, 'B' },
    { "recover", no_argument, NULL, 'R' },
    { "key", required_argument, NULL, 'k' },
    { "opt", required_argument, NULL, 'o' },
    { "config", required_argument, NULL, 'f' },
    { "profile", required_argument, NULL, 'p' },
    { "machine", required_argument, NULL, 'm' },
    { "ftype", required_argument, NULL, 't' },
    { "fversion", required_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

void display_usage(void);
void show_result(const char*);
int daemon_init(void);
void dump_params(struct Config cf);    
bool GetBlackDisConf(const int profile_id,vector<profile_sid>& psids,vector<psm>& psms);
bool GetBlackProfileToDis(const vector<profile_sid> & psids,vector<int>& profile_ids);


////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
    if (argc == 1)
    {
        display_usage();
    }
    
//PARSE PARAM///////////////////////////////////////////////////////////////////////////////////////////
    int opt = 0;
    int long_index = 0;

    opt = getopt_long( argc, argv, opt_string, long_opts, &long_index );
    while (opt != -1)
    {
        switch(opt)
        {
            case 'L':
                global_config.do_list = 1;
                break;
            case 'D':
                global_config.do_daemon = 1;
                break;
            case 'A':
                global_config.do_all = 1; 
                break;
            case 'B':
                global_config.do_blacklist = 1; 
                break;
            case 'R':
                global_config.do_recover = 1; 
                break;
            case 'o':
                global_config.opt = optarg;
                break;
            case 'k':
                global_config.key = optarg;
                break;
            case 'f':
                global_config.config = optarg;
                break;
            case 'p':
                global_config.profile_id = atoi(optarg);
                break;
            case 'm':
                global_config.machine_id = atoi(optarg);
                break;
            case 't':
                global_config.conf_file_type = atoi(optarg);
                break;
            case 'v':
                global_config.conf_file_version = atoi(optarg);
                break;
            case 'h':
            case '?':
            case 0:
                display_usage();
                break;
            default:
                break;
        }

        opt = getopt_long( argc, argv, opt_string, long_opts, &long_index );
    }

///INIT /////////////////////////////////////////////////////////////////////////////////////////
    struct Config cf;

    if (global_config.config == "")
    {
        global_config.config = "./distribute.cf";
    }
    
    CXmlConfig cxc(global_config.config);
    if (false == cxc.GetConfig(cf))
    {
        cerr << "Parse Config file " << global_config.config 
                << " Failed,exit!" << endl;
        exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    dump_params(cf);    
#endif

    if (global_config.do_daemon == 1)
    {
        daemon_init();
    }
    
    CDBOption* dbopt = new CDBOption(cf.oracle_user + "/" + cf.oracle_pass 
                        + "@" + cf.oracle_sid);
    CDistribute* distribute = new CDistribute(cf.cms_ip.c_str(),cf.cms_port);
    CMachine* machine = new CMachine();
    CDisFile* file = new CDisFile();
    CRunLog* xlog = new CRunLog();
    xlog->SetLogFileName((char*)cf.log_file.c_str());

    distribute->setCmsStorePath(cf.cms_file_storge_path);
    distribute->setDistributePath(cf.distribute_file_path);

	
//LIST //////////////////////////////////////////////////////////////////////////////////////////
    if (global_config.do_list == 1)
    {
        xlog->WriteRunLog("get all machines start ...");

        cout << distribute->getTotalList() << endl;
        xlog->WriteRunLog("%s",distribute->getTotalList().c_str());

        xlog->WriteRunLog("get all machines end ...");

		delete file;
		delete machine;
		delete distribute;
		delete dbopt;
		delete xlog;

        return 0;
    }

//BLACKLIST RECOVER ////////////////////////////////////////////////////////////////////////////
	if (global_config.do_blacklist == 1 && global_config.do_recover == 1)
	{
		if (global_config.profile_id > 0)
		{
			vector<string> ips;
			if (false == dbopt->GetBlackListByProfile(global_config.profile_id,ips))
			{
				cerr << "GetBlackListByProfile Failed" << endl;
				xlog->WriteRunLog("GetBlackListByProfile Failed");
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 1;
			}

			if (ips.size() == 0)
			{
				cerr << "ips size is 0" << endl;
				xlog->WriteRunLog("ips size (profile: %d) is 0",global_config.profile_id);
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 0;
			}

			vector<psm> psms;

			if (false == GetBlackDisConf(global_config.profile_id,cf.psids,psms))
			{
				cerr << "GetBlackDisConf Failed!" << endl;
				xlog->WriteRunLog("GetBlackDisConf Failed!");
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 1;
			}

			stringstream to_dist_file;
			to_dist_file << global_config.profile_id;
			to_dist_file << "-";
			to_dist_file << time(NULL);

			for (vector<psm>::iterator i = psms.begin(); i != psms.end(); ++i)
			{
				CBuildBlackList* buildblfile = new CBuildBlackList((cf.distribute_file_path + to_dist_file.str()).c_str());
				if (false == buildblfile->BuildBlackList(ips))
				{
					cerr << "BuildBlackList (profile_id:" << global_config.profile_id << endl;
					xlog->WriteRunLog("BuildBlackList (profile_id:%d) Failed!",
						global_config.profile_id);
					continue;
				}

				delete buildblfile;

				if (false == distribute->distributeBL(i->mem,i->sid,to_dist_file.str()))
				{
					cerr << "distribute " << to_dist_file.str() << " Failed!" << endl;
					xlog->WriteRunLog("distribute %s Failed!",to_dist_file.str().c_str());
					continue;
				}
			}
		}
		else
		{
			vector<int> bl_profile_ids;
			if (false == GetBlackProfileToDis(cf.psids,bl_profile_ids))
			{
				cerr << "GetBlackProfileToDis Failed!" << endl;
				xlog->WriteRunLog("GetBlackProfileToDis Failed!");
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 1;
			}
			
			for (vector<int>::iterator s = bl_profile_ids.begin(); s != bl_profile_ids.end(); s++)
			{
				vector<string> ips;
				if (false == dbopt->GetBlackListByProfile(*s,ips))
				{
					cerr << "GetBlackListByProfile Failed" << endl;
					xlog->WriteRunLog("GetBlackListByProfile Failed");
					continue;
				}

				if (ips.size() == 0)
				{
					cerr << "ips size is 0" << endl;
					xlog->WriteRunLog("ips size (profile: %d) is 0",*s);
					continue;
				}

				vector<psm> psms;
				if (false == GetBlackDisConf(*s,cf.psids,psms))
				{
					cerr << "GetBlackDisConf Failed!" << endl;
					xlog->WriteRunLog("GetBlackDisConf Failed!");
					continue;
				}

				stringstream to_dist_file;
				to_dist_file << *s;
				to_dist_file << "-";
				to_dist_file << time(NULL);

				for (vector<psm>::iterator i = psms.begin(); i != psms.end(); ++i)
				{
					CBuildBlackList* buildblfile = new CBuildBlackList((cf.distribute_file_path + to_dist_file.str()).c_str());
					if (false == buildblfile->BuildBlackList(ips))
					{
						cerr << "BuildBlackList (profile_id:" << *s  << endl;
						xlog->WriteRunLog("BuildBlackList (profile_id:%d) Failed!",
							*s);
						continue;
					}

					delete buildblfile;

					if (false == distribute->distributeBL(i->mem,i->sid,to_dist_file.str()))
					{
						cerr << "distribute " << to_dist_file.str() << " Failed!" << endl;
						xlog->WriteRunLog("distribute %s Failed!",to_dist_file.str().c_str());
						continue;
					}
				}
			}
		}

		delete file;
		delete machine;
		delete distribute;
		delete dbopt;
		delete xlog;

		return 0;
	}

//BLACKLIST ///////////////////////////////////////////////////////////////////////////////////
	if (global_config.do_blacklist == 1)
	{
		if (global_config.key != "")
		{
			
			vector<psm> psms;

			if (false == GetBlackDisConf(global_config.profile_id,cf.psids,psms))
			{
				cerr << "GetBlackDisConf Failed!" << endl;
				xlog->WriteRunLog("GetBlackDisConf Failed!");
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 1;
			}


			for (vector<psm>::iterator i = psms.begin(); i != psms.end(); ++i)
			{
				if (false == distribute->setTapBlackList(i->mem,global_config.opt,global_config.key,i->sid))
				{
					cerr << "setTapBlackList Failed!" << endl;
					xlog->WriteRunLog("setTapBlackList Failed!");
					continue;
				}
			}

		}
		else  //auto do all
		{
			
			vector<int> bl_profile_ids;
			if (false == GetBlackProfileToDis(cf.psids,bl_profile_ids))
			{
				cerr << "GetBlackProfileToDis Failed!" << endl;
				xlog->WriteRunLog("GetBlackProfileToDis Failed!");
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
				return 1;
			}

			for (vector<int>::iterator i = bl_profile_ids.begin(); i != bl_profile_ids.end(); ++i)	
			{
				vector<psm> psms;

				if (false == GetBlackDisConf(*i,cf.psids,psms))
				{
					cerr << "GetBlackDisConf Failed!" << endl;
					xlog->WriteRunLog("GetBlackDisConf Failed!");
					continue;
				}

				vector<int> machine_ids;
				if (false == dbopt->GetMachineID(*i,machine_ids))
				{
					cerr << "profile " << *i << "get machine failed!" << endl;
					xlog->WriteRunLog("profile (%d) get machine failed!",*i);
					continue;
				}

            	for (vector<int>::iterator j = machine_ids.begin(); j != machine_ids.end(); j++)
				{
					
                    *machine = dbopt->GetMachine(*j);
                    if (machine->getID() == -1)
                    {
                        cerr << "dbopt->GetMachine id(" << *j << ") Failed!" << endl;
                        xlog->WriteRunLog("dbopt->GetMachine id(%d) Failed!",*j);
                        continue;
                    }

					if (false == distribute->getTapBlackListFile(*machine))
					{
						cerr << "getTapBlackListFile (" << *j << ") Failed!" <<endl;
						xlog->WriteRunLog("getTapBlackListFile(%s) Failed!",machine->getName().c_str());
						continue;
					}
					else
					{
						vector<blacklist> blackLists;
						if (false == distribute->parseTapBlackListFile(blackLists))
						{
							cerr << "parseTapBlackListFile Failed!" << endl;
							xlog->WriteRunLog("parseTapBlackListFile Failed!");
							continue;
						}

						for (vector<blacklist>::iterator k = blackLists.begin(); k != blackLists.end(); ++k)
						{
							unsigned int tag = 0;
							cout << k->ip << ":" << k->operation << ":" << k->creator << endl;
							for (vector<psm>::iterator m = psms.begin(); m != psms.end(); ++m)
							{
								if (false == distribute->setTapBlackList(m->mem,"set",k->ip,m->sid))
								{
									tag++;
									cerr << "setTapBlackList Failed!" << endl;
									continue;
								}
							}
							if (tag < psms.size())
							{
								if (false == dbopt->SetBlackList(*i,k->ip,k->operation,k->creator,
										machine->getName(),GetCurrentTime(),"AutoAdd"))
								{
									cerr << "SetBlackList Failed!" << endl;
									continue;
								}
							}
						}
					}
				} // machine_ids loop
			}//profile_ids loop
			
		}
	}

//REGULAR MANUAL///////////////////////////////////////////////////////////////////////////////////////////
    if (global_config.do_all != 1 && global_config.do_blacklist == -1)
    {
        xlog->WriteRunLog("distribute single file to single machine start ...");

        if (global_config.profile_id < 0 || global_config.machine_id < 0 ||
            global_config.conf_file_type < 0)
        {
            display_usage();
        }
        
        if (global_config.conf_file_version >= 0)
        {
            file->setType(global_config.conf_file_type);
            file->setVersion(global_config.conf_file_version);
            file->setProfileID(global_config.profile_id);
        }
        else
        {
            *file = distribute->getMaxVersionFile(global_config.profile_id,
                                                    global_config.conf_file_type);
            if (file->getVersion() == -1)
            {
                cerr << "distribute->getMaxVersionFile type(" << global_config.profile_id 
                        << ":" << global_config.conf_file_type << ") Failed!" << endl;
                xlog->WriteRunLog("distribute->getMaxVersionFile type(%d:%d) Failed!",
                                    global_config.profile_id,global_config.conf_file_type);
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
                return 1;
            }
        }

        *machine = dbopt->GetMachine(global_config.machine_id);
        if (machine->getID() == -1)
        {
            cerr << "dbopt->GetMachine id(" << global_config.machine_id << ") Failed!" << endl;
            xlog->WriteRunLog("dbopt->GetMachine id(%d) Failed!",global_config.machine_id);

			delete file;
			delete machine;
			delete distribute;
			delete dbopt;
			delete xlog;
            return 1;
        }

        if (false == distribute->distribute(*machine,*file))
        {
            cerr << "distribute " << file->getFileName() << "failed !" << endl;
            xlog->WriteRunLog("distribute %s to machine %d Failed!",
                file->getFileName().c_str(),global_config.machine_id);
			delete file;
			delete machine;
			delete distribute;
			delete dbopt;
			delete xlog;
            return 1;
        }
        else
        {
            if (false == dbopt->SetMachineVersion(global_config.profile_id,global_config.machine_id,
                file->getType(),file->getVersion()))
            {
                cerr << "SetMachineVersion" << file->getFileName() << "failed!" << endl;
                xlog->WriteRunLog("SetMachineVersion %d %s",global_config.machine_id,
                                        file->getFileName().c_str());
				delete file;
				delete machine;
				delete distribute;
				delete dbopt;
				delete xlog;
                return 1;
            }
        }
        
        if (false == distribute->remoteExec(*machine,"TapReload"))
        {
            cerr << "TapReload machine id(" << global_config.machine_id << ") Failed!" << endl;
            xlog->WriteRunLog("TapReload machine id(%d %s) Failed!",
                    global_config.machine_id,machine->getName().c_str());
			delete file;
			delete machine;
			delete distribute;
			delete dbopt;
			delete xlog;
            return 1;
        }

        xlog->WriteRunLog("distribute single file to single machine end ...");
    }
    
//REGULAR AUTO//////////////////////////////////////////////////////////////////////////////////////////
    if (global_config.do_all == 1)
    {
        xlog->WriteRunLog("distribute all files to all profiles (machines) start ...");

        vector<int> profile_ids;

        if (false == dbopt->GetProfileID(profile_ids))
        {
            cerr << "getProfileID Failed!" << endl; 
            xlog->WriteRunLog("getProfileID Failed!");
			delete file;
			delete machine;
			delete distribute;
			delete dbopt;
			delete xlog;
            return 1;
        }

        for (vector<int>::iterator i = profile_ids.begin(); i != profile_ids.end(); i++)
        {
            vector<int> machine_ids;
            if (false == dbopt->GetMachineID(*i,machine_ids))
            {
                cerr << "profile " << *i << "get machine failed!" << endl;
                xlog->WriteRunLog("profile (%d) get machine failed!",*i);
                continue;
            }
            
            vector<int> types;
            if (false == dbopt->GetTypes(*i,types))
            {
                cerr << "profile " << *i << "get types failed!" << endl;
                xlog->WriteRunLog("profile (%d) get types failed!",*i);
                continue;
            }

            for (vector<int>::iterator j = machine_ids.begin(); j != machine_ids.end(); j++)
            {
                for (vector<int>::iterator k = types.begin(); k != types.end(); k++)
                {
					if (*k == BLACKLIST_TYPE) continue; //blacklist except

                    *machine = dbopt->GetMachine(*j);
                    if (machine->getID() == -1)
                    {
                        cerr << "dbopt->GetMachine id(" << *k << ") Failed!" << endl;
                        xlog->WriteRunLog("dbopt->GetMachine id(%d) Failed!",*k);
                        continue;
                    }

                    *file = distribute->getMaxVersionFile(*i,*k);
                    if (file->getVersion() == -1)
                    {
                        cerr << "distribute->getMaxVersionFile type(" << *i << ":" 
                                <<  *k << ") Failed!" << endl;
                        xlog->WriteRunLog("distribute->getMaxVersionFile type(%d:%d) Failed!",*i,*k);
                        continue;
                    }
                    
                    int machine_type_version = dbopt->GetMachineVersion(*i,*j,*k);
                    if (machine_type_version == -1 || machine_type_version >= file->getVersion())
                    {
                        cerr << "already newest,no update (" << machine_type_version << ":" 
                            << file->getVersion() << ")" << endl;
                        xlog->WriteRunLog("already newest,no update (%d:%d)",machine_type_version,file->getVersion());
                        continue;
                    }

                    if (false == distribute->distribute(*machine,*file))
                    {
                        cerr << "distribute " << file->getFileName() << "failed !" << endl;
                        xlog->WriteRunLog("distribute (%d) %s failed!",*i,file->getFileName().c_str());
                        continue;
                    }
                    else
                    {
                        if (false == dbopt->SetMachineVersion(global_config.profile_id,global_config.machine_id,
                            file->getType(),file->getVersion()))
                        {
                            cerr << "SetMachineVersion" << file->getFileName() << "failed!" << endl;
                            xlog->WriteRunLog("SetMachineVersion (%d) %s failed!",*i,file->getFileName().c_str());
                            continue;
                        }
                    }

                } 

                if (false == distribute->remoteExec(*machine,"TapReload"))
                {
                    cerr << "TapReload machine id(" << *j << ") Failed!" << endl;
                    xlog->WriteRunLog("TapReload machine id(%d%s) Failed!",*j,machine->getName().c_str());
                    continue;
                }
            }
        }

        xlog->WriteRunLog("distribute all files to all profiles (machines) end ...");
    }

    dbopt->Commit();
    delete file;
    delete machine;
    delete distribute;
    delete dbopt;
    delete xlog;

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void display_usage( void )
{
    puts("distribute - distribute config file to tap");
    puts("  --help|-h)\t\tshow this messenge");
    puts("  --daemon|-D)\t\trun in background");
    puts("  --all|-A)\t\tauto distribute all config file");
    puts("  --blacklist|-B)\tblacklist");
    puts("  --recover|-R)\t\tblacklist recover");
    puts("  --list|-L)\t\tshow all machines online");
    puts("  --config|-f)\t\tthe config params file");
    puts("  --key|-k)\t\tthe blacklist value");
    puts("  --opt|-o)\t\tthe blacklist option <add/set/delete/replace>");
    puts("  --profile|-p)\t\tto distribute profile id");
    puts("  --machine|-m)\t\tto distribute machine id");
    puts("  --ftype|-t)\t\tto distribute file type");
    puts("  --fversion|-v)\tto distribute file version");
    exit( EXIT_FAILURE );
}

void dump_params(struct Config cf)
{
    cout << "==========PARAMS==============" << endl;
    cout << "config:    " << global_config.config << endl;
    cout << "profile:   " << global_config.profile_id << endl;
    cout << "ftype:     " << global_config.conf_file_type << endl;
    cout << "fversion:  " << global_config.conf_file_version << endl;   
	cout << "key:		" << global_config.key << endl;
    cout << "db:        " << cf.oracle_user << "/" << cf.oracle_pass 
            << "@" << cf.oracle_sid << endl;
    cout << "CMS:       " << cf.cms_ip << ":" << cf.cms_port << "->"
            << cf.cms_file_storge_path << endl;
    cout << "distribute:" << cf.distribute_file_path << endl;
    cout << "log:       " << cf.log_file << endl;
    cout << "==========PARAMS==============" << endl;
}

void show_result(const char* res)
{
    if (NULL != res)
    {
        fprintf(stdout,"\n%s\n",res);
    }
}

int daemon_init(void)
{
	int		i;
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			

	if (setsid() < 0)			
		return (-1);

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		_exit(0);			

	chdir("/");			

	for (i = 0; i < 64; i++)
		close(i);

	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	return (0);				
}

bool GetBlackDisConf(const int profile_id,vector<profile_sid>& psids,vector<psm>& psms)
{
	psms.clear();
	for (vector<profile_sid>::iterator i = psids.begin(); i != psids.end(); ++i)
	{
		if (i->profile_id == profile_id)
		{
			for (vector<psm>::iterator j = i->psms.begin(); j != i->psms.end(); ++j)
			{
				psms.push_back(*j);
			}
			return true;
		}
	}

	return false;
}

bool GetBlackProfileToDis(const vector<profile_sid> & psids,vector<int>& profile_ids)
{
	profile_ids.clear();
	
	for (vector<profile_sid>::const_iterator i = psids.begin(); i != psids.end(); ++i)
	{
		profile_ids.push_back(i->profile_id);
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////
