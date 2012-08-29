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

  private:
    otl_connect* m_conn;
    otl_stream* m_stream;
};

#endif /*_CDBOPTION_H_*/
