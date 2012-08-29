#ifndef _CDBOPTION_H_
#define _CDBOPTION_H_

#include <string>
#include <cstring>
#include <vector>

#ifndef OTL_ORA10G
#define OTL_ORA10G
#endif

#define OTL_STREAM_READ_ITERATOR_ON
#define OTL_STL

#include "otlv4.h"

using std::string;
using std::vector;

#include "CProfile.h"
#include "CMachine.h"
#include "Common.h"

struct blackip
{
	int profile_id;
	string ip;
	string operation;
	string creator;
	string sid;
	string create_time;
	string remark;

	blackip()
	{
		profile_id = 0;
		ip = " ";
		operation = "REJECT";
		creator = "TAP";
		sid = "DUMP";
		create_time = GetCurrentTime();
		remark = "AutoAdd";
	}
};

class CDBOption
{
  public:
    CDBOption (const string conn);
     ~CDBOption ();

    void WriteError (const char *type, otl_exception & e);
    void Commit ();

    bool GetProfileID(vector<int> &profile_ids);
    bool GetMachineID(int profile_id,vector<int> &machine_ids);
    bool GetTypes(int profile_id,vector<int> &types);

    CMachine GetMachine(int machine_id);

    bool SetMachineVersion(int profile_id,int machine_id,int type,int version);
    bool SetProfileVersion(int profile_id,int type,int version);

    int GetMachineVersion(int profile_id,int machine_id,int type);

	//Tap BlackList Conf
	bool GetBlackProfileToDis(vector<int>& bl_profile_ids);
	bool GetBlackDisConf(const int profile_id,string& sid,string& mem);

	bool SetBlackList(const int profile_id,const string& ip,const string& operation,
				const string& creator,const string& sid,
				const string& create_time,const string& remark);

	bool DumpBlackList(const vector<struct blackip>& ips);

	bool GetBlackListByProfile(const int profile_id,vector<string>& ips);

  private:
    otl_connect* m_conn;
    otl_stream* m_stream;
};

#endif /*_CDBOPTION_H_*/
