#include "CDBOption.h"
#include "Common.h"

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <fstream>
using std::fstream;

string db_conn = "tapm/tapm263@192.168.188.16:1521/orcl";

int main(int argc,char* argv[])
{
	if (argc != 5)
	{
		cerr << "Usage: " << argv[0] << " <profile_id> <filename> <split(0:\\t,1: )> <db_conn>" << endl;
		exit(1);
	}

    char split[2] = {'\0'};

    if (atoi(argv[3]) == 0)
    {   
        strcpy(split,"\t");
    }   
    else if (atoi(argv[3]) ==1)
    {   
		strcpy(split," ");
    }
	else
	{
		cerr << "Usage: " << argv[0] << " <profile_id> <filename> <split(0:\\t,1: )>" << endl;
		exit(1);
	}

	int profile_id = atoi(argv[1]);

	vector<struct blackip> blackLists;
	blackLists.clear();

	fstream fp(argv[2]);
	string line_buffer;

	db_conn = string(argv[4]);
	cout << "db_conn:" << db_conn << endl;

	CDBOption myDBOpt(db_conn);


	int count = 1;
	while(getline(fp,line_buffer))
	{
		count++;

		blackip buffer;
		buffer.profile_id = profile_id;
		string::size_type pos1 = line_buffer.find(split);
		if (pos1 != string::npos)
		{
			buffer.ip = line_buffer.substr(0,pos1-0);
			string::size_type pos2 = line_buffer.find(split,++pos1);
			if (pos2 != string::npos)
			{
				buffer.operation = line_buffer.substr(pos1,pos2-pos1);
				string::size_type pos3 = line_buffer.find(split,++pos2);
				if (pos3 != string::npos)
				{		
					buffer.creator = line_buffer.substr(pos2,pos3-pos2);
					string::size_type pos4 = line_buffer.find(split,++pos3);
					if (pos4 != string::npos)
					{
						buffer.create_time = line_buffer.substr(pos3,pos4-pos3);
						if (++pos4 != line_buffer.size()) buffer.remark = line_buffer.substr(pos4,string::npos);
					}
				}
			}

			blackLists.push_back(buffer);
		}

		if (count % 1000 == 0)
		{
			cout << count << endl;
			if (false == myDBOpt.DumpBlackList(blackLists))
			{
				cerr << "Dump error!" << " count=" << count << endl;
			}
			else
			{
				myDBOpt.Commit();
			}

			blackLists.clear();
		}
	}

	if (false == myDBOpt.DumpBlackList(blackLists))
	{
		cerr << "Dump error!" << " count=" << count << endl;
	}
	else
	{
		myDBOpt.Commit();
		blackLists.clear();
	}

	exit(0);
}
