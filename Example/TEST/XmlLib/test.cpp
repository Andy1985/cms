#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

#include "tinyxml.h"
//#include "Socket.hpp"

#include <iostream>
#include <fstream>
#include <string>

#define FRAME_SIZE 20
#define FRAME_FILL 'A'

using std::cout;
using std::endl;
using std::string;
using std::cerr;

void xml2string(TiXmlDocument &myDoc,string &data)
{
	
	if (&myDoc != NULL)
	{
		myDoc.SaveFile("./temp.xml");
	}

	FILE* mFile = fopen("./temp.xml", "r");
	struct stat st;
	fstat(fileno(mFile), &st);

	unsigned char* p = new unsigned char[st.st_size+1];
	memset(p,0,st.st_size+1);
	fread(p, sizeof(char), st.st_size, mFile);
	data.clear();
	data.append((const char*)p);
	fclose(mFile);
	unlink("./temp.xml");

}

void build_send_data(string &data)
{
	//char frame[FRAME_SIZE] = {'A'};
	string buffer;

	TiXmlDocument *myDoc = new TiXmlDocument();
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0","gb2312","yes");
	myDoc->LinkEndChild(decl);
	TiXmlElement *root = new TiXmlElement("Msg");
	root->SetAttribute("TaskType","ADDRGET");
	myDoc->LinkEndChild(root);
	TiXmlElement *param = new TiXmlElement("AddrParam");
	param->SetAttribute("Time","2010-11-15 09:42:30");
	root->LinkEndChild(param);
	TiXmlElement *sender = new TiXmlElement("SnderAcc");
	sender->SetAttribute("NameValue","xueming.li@net263.com");
	param->LinkEndChild(sender);
	TiXmlElement *recever = new TiXmlElement("RecipAcc");
	recever->SetAttribute("NameValue","test@test.com");
	param->LinkEndChild(recever);

	myDoc->ToString(data);

	delete myDoc;
}

int main(void)
{
		/*
	SocketEx sock;
	char *host = "127.0.0.1";
	unsigned int port = 12000;

	if (! sock.Create(false))
	{
		cerr << "Create socket failed!" << endl;
		return -1;
	}

	if (! sock.Connect(host,port))
	{
		cerr << "Can not connect to " << host << ":" << port << "!" << endl;
		sock.Close();
		return -1;
	}

	cout << "Connect to oracle!" << endl;
	
	sock.Close();
	
	string data;

	build_send_data(data);

	cout << data << endl;

	if (false == myDoc->Parse(result))
	{
		cerr << "Parse " << result << " error!" << endl;
		return false;
	}
	char _result[] =  "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>"
					"<Msg TaskType=\"ADDRGET\">" 
						"<AddrGet TaskType=\"Rsp\" TaskId=\"1\" MachineId= \"1\" Result=\"OK\"/>"
					"</Msg>";
	*/
	char *result = "<root>test</root>";
	TiXmlDocument *myDoc = new TiXmlDocument();
	if (false == myDoc->Parse(result))
	{
		cerr << "Parse " << result << " error!" << endl;
	//	return -1;
	}
	

    TiXmlElement *root = myDoc->RootElement ();
	cout << root->GetText() << endl;

	delete myDoc;
	return 0;
	if (NULL == root) return false;
    TiXmlElement *AddrGet = root->FirstChildElement ();
	if (NULL == AddrGet) return false;
	TiXmlAttribute *att = AddrGet->FirstAttribute();
	if (NULL == att) return false;

    while (NULL != att)
    {
		if (0 == strcmp(att->Name(),"Result") && 0 == strcmp(att->Value(),"OK"))
		{
			return true;
		}
        att = att->Next();
    }

	delete myDoc;

	return 0;
}
