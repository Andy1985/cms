#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "CDBOption.h"

const unsigned int MAX_SQL_SIZE = 1024 * 1024 * 2; // 2M

using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::vector;
using std::stringstream;

CDBOption::CDBOption (const string conn)
{
    try
    {
        otl_connect::otl_initialize (1);
        m_conn = new otl_connect;
        m_stream = new otl_stream;
		m_conn->rlogon(conn.data(),0);
		m_conn->auto_commit_off();
    }
    catch (otl_exception & p)
    {
        WriteError ("Constuctor: ", p);
    }
}

CDBOption::~CDBOption ()
{
    m_conn->logoff();
    delete m_stream;
    delete m_conn;
}

void CDBOption::WriteError (const char *type, otl_exception & e)
{
    cerr << type << " " << e.msg << " " << e.stm_text << " " << e.var_info << endl;
}

void CDBOption::Commit ()
{
    try
    {
        m_conn->commit ();
    }
    catch (otl_exception & p)
    {
        WriteError ("Commit: ", p);
    }
}

bool CDBOption::GetProfileID(vector<int> &profile_ids)
{
    string query = "select id from profile";
    profile_ids.clear();

    try
    {
        int szData = 0;

        m_stream->open(1,query.data(),*m_conn);

        otl_stream_read_iterator < otl_stream, otl_exception,
            otl_lob_stream > rs;

        rs.attach (*m_stream);
    
        while (rs.next_row ())
        {
            rs.get ("id", szData);
            profile_ids.push_back (szData);
        }

        rs.detach ();
        m_stream->close();

        return true;
    }
    catch (otl_exception & p)
    {
        m_stream->close();
        this->WriteError ("GetProfileID Error: ", p);
        return false;
    }
}

bool CDBOption::GetMachineID(int profile_id,vector<int> &machine_ids)
{
    stringstream query;
    query << "select id from profile_machine where profile_id = ";
    query << profile_id;

    try
    {
        int szData = 0;

        m_stream->open(1,query.str().data(),*m_conn);

        otl_stream_read_iterator < otl_stream, otl_exception,
            otl_lob_stream > rs;

        rs.attach (*m_stream);
    
        while (rs.next_row ())
        {
            rs.get ("id", szData);
            machine_ids.push_back (szData);
        }

        rs.detach ();
        m_stream->close();

        return true;
    }
    catch (otl_exception & p)
    {
        m_stream->close();
        this->WriteError ("GetMachineID Error: ", p);
        return false;
    }
}

CMachine CDBOption::GetMachine(int machine_id)
{
    stringstream query;
    query << "select profile_id,ip from profile_machine where id = ";
    query << machine_id;

    CMachine buffer;

    try
    {
        int szData = 0;
        char szData1[128] = {'\0'};

        m_stream->open(1,query.str().data(),*m_conn);

        otl_stream_read_iterator < otl_stream, otl_exception,
            otl_lob_stream > rs;

        rs.attach (*m_stream);
    
        while (rs.next_row ())
        {
            rs.get ("profile_id", szData);
            rs.get ("ip",szData1);
        }

        rs.detach ();
        m_stream->close();

        stringstream name;
        name << szData;
        name << "_";
        name << string(szData1);

        buffer.setID(machine_id);
        buffer.setName(name.str());

        return buffer;
    }
    catch (otl_exception & p)
    {
        m_stream->close();
        this->WriteError ("GetMachine Error: ", p);
    }

    return buffer;
}

bool CDBOption::GetTypes(int profile_id,vector<int> &types)
{
    stringstream query;
    query << "select distinct type from profile_version where profile_id = ";
    query << profile_id;

    try
    {
        int szData = 0;

        m_stream->open(1,query.str().data(),*m_conn);

        otl_stream_read_iterator < otl_stream, otl_exception,
            otl_lob_stream > rs;

        rs.attach (*m_stream);
    
        while (rs.next_row ())
        {
            rs.get ("type", szData);
            types.push_back (szData);
        }

        rs.detach ();
        m_stream->close();

        return true;
    }
    catch (otl_exception & p)
    {
        m_stream->close();
        this->WriteError ("GetTypes Error: ", p);
        return false;
    }
}

bool CDBOption::SetProfileVersion(int profile_id,int type,int version)
{
    try
    {
        char strSql[] = "BEGIN "
                        "sp_profile_version_update(:profile_id<int,in>,:type<int,in>,:version<int,in>); "
                        "END;";

        m_stream->open(1,strSql,*m_conn);

        *m_stream << profile_id << type << version;

        m_stream->close();
        return true;
    } 
    catch (otl_exception &p)
    {
        m_stream->close();
        this->WriteError("SetProfileVersion Error: ",p);
        return false;
    }
}

bool CDBOption::SetMachineVersion(int profile_id,int machine_id,int type,int version)
{
    try
    {
        char strSql[] = "BEGIN "
                        "sp_machine_version_update(:profile_id<int,i>,:profile_machine_id<int,in>,:type<int,in>,:version<int,in>); "
                        "END;";
        m_stream->open(1,strSql,*m_conn);

        *m_stream << profile_id << machine_id << type << version;

        m_stream->close();
        return true;
    } 
    catch (otl_exception &p)
    {
        m_stream->close();
        this->WriteError("SetMachineVersion Error: ",p);
        return false;
    }
}
