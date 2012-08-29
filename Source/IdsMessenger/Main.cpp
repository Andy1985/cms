// Main.cpp
#include <map>
#include <string>
#include <csignal>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "../Common.h"
#include "../../Supports/MiniLib/String.hpp"
#include "../../Supports/MiniLib/SocketSSL.hpp"

time_t check_time;
int sending_data_tag;

///////////////////////////////////////////////////////////////////////////

std::string transferFolder = "./";

std::map<std::string, std::string> rules;

bool LoadRules(const char* filename)
{
	if ( ! filename) return false;

	FILE* fp = fopen(filename, "r");
	if ( ! fp) return false;

	std::string curCmd = "";
	while ( ! feof(fp))
	{
		char buffer[1024] = "";
		if (fgets(buffer, sizeof(buffer), fp))
		{
			std::string s = buffer;
			if ((s.size() > 0) && (s.substr(s.size() - 1) == "\n"))
			{
				s = s.substr(0, s.size() - 1);
			}
			if ((s.size() > 0) && (s.substr(s.size() - 1) == "\r"))
			{
				s = s.substr(0, s.size() - 1);
			}
			if (s.empty()) continue;

			if (s[0] == '#') continue;

			if (s[0] != '\t')
			{
				size_t pos = s.find_first_of(':');
				if (pos != std::string::npos)
				{
					s = s.substr(0, pos);
				}
				curCmd = s;
			}
			else
			{
				s = s.substr(1);
				if ( ! curCmd.empty())
				{
					if (rules.find(curCmd) == rules.end())
					{
						rules[curCmd] = s;
					}
					curCmd = "";
				}
			}
		}
	}
	
	std::cout << "All " << rules.size() << " rule(s) loaded." << std::endl;
	return true;
}

std::string CreateJSON(int error, const std::string& message, const std::string& resultJSON = "")
{
	std::stringstream ss;
	ss << "{" << std::endl;

	ss << " \"error\":" << error << "," << std::endl;
	ss << " \"message\":\"" << message << "\"," << std::endl;
	
	ss << " \"result\":";
	if (resultJSON.empty())
	{
		ss << "{}" << std::endl;
	}
	else
	{
		ss << resultJSON << std::endl;
	}

	ss << "}";
	return ss.str();
}

bool RunScript(const std::vector<char>& data, std::vector<char>& result)
{
	result.clear();
	std::string text(data.begin(), data.end());

	std::cout << "Run script:" << std::endl << text << std::endl;

	std::vector<std::string> lines = string_split(text, '\n');
	if ( ! lines.empty())
	{
		std::string cmd = lines[0];
		std::map<std::string, std::string> params;
		for (size_t i = 1; i < lines.size(); ++i)
		{
			size_t pos = lines[i].find_first_of(':');
			if (pos != std::string::npos)
			{
				std::string name = lines[i].substr(0, pos);
				std::string value = lines[i].substr(pos + 1);
				if ( ! name.empty())
				{
					params[name] = value;
				}
				std::cout << "  param: " << name << " => " << value << std::endl;
			}
		}

		if (rules.find(cmd) == rules.end())
		{
			std::cerr << "Unknown command: '" << cmd << "'" << std::endl;
		}
		else
		{
			std::string command = rules[cmd];
			for (std::map<std::string, std::string>::iterator it = params.begin(); it != params.end(); ++it)
			{
				std::cout << " Replace " << it->first << " => " << it->second << std::endl;

				command = string_replace(command, "$(" + it->first + ")", it->second);
			}
			command = string_replace(command, "{", "\\{");
			command = string_replace(command, "}", "\\}");
			command = string_replace(command, "\"", "\\\"");
			command = string_replace(command, "$", "\\$");
			std::cout << "Call: " << rules[cmd] << std::endl;
			std::cout << "Run: " << command << std::endl;

			FILE* fp = popen(command.c_str(), "r");
			if ( ! fp)
			{
				std::cerr << "Run script failed!" << std::endl;
				std::string text = CreateJSON(1, "Can not run script!");
				result.insert(result.begin(), text.begin(), text.end());
			}
			else
			{
				std::string text = "";
				while ( ! feof(fp))
				{
					char buffer[1024];
					while (fgets(buffer, sizeof(buffer), fp))
					{
						text += buffer;
					}
				}
				fclose(fp);
				result.insert(result.end(), text.begin(), text.end());
			}
		}
	}
	return true;
}

bool ProcessSend(SocketSSLEx& sock, std::vector<char>& result)
{
	std::vector<char> data;
	if ( ! sock.RecvPackage(data))
	{
		std::cerr << "Read data for Send command failed!" << std::endl;
		return false;
	}
	std::cout << "Command Data (" << data.size() << " bytes) received." << std::endl;

	return RunScript(data, result);
}

bool ProcessDownload(SocketSSLEx& sock, std::vector<char>& result)
{
	std::vector<char> filenameData;
	if ( ! sock.RecvPackage(filenameData))
	{
		std::cerr << "Read filename for File command failed!" << std::endl;
		return false;
	}
	std::string filename = transferFolder + "/" + std::string(filenameData.begin(), filenameData.end());

	FILE* fp = fopen(filename.c_str(), "rb");
	int resultInt = ( ! fp);
	if (sizeof(resultInt) != sock.SendEx(&resultInt, sizeof(resultInt)))
	{
		std::cerr << "Send result int failed!" << std::endl;
		return false;
	}
	if ( ! fp)
	{
		std::cerr << "Open file '" << filename << "' failed!" << std::endl;
		return true;
	}

	fseek(fp, 0, SEEK_END);
	size_t fileSize = static_cast<size_t>(ftell(fp));
	fseek(fp, 0, SEEK_SET);
	std::cout << "File size: " << fileSize << std::endl;

	if (sizeof(fileSize) != sock.SendEx(&fileSize, sizeof(fileSize)))
	{
		std::cerr << "Send file size failed!" << std::endl;
		fclose(fp);
		return false;
	}

	while ( ! feof(fp))
	{
		char buffer[32 * 1024];
		size_t size = fread(buffer, 1, sizeof(buffer), fp);
		if (static_cast<int>(size) != sock.SendEx(buffer, size))
		{
			std::cerr << "Send file data failed!" << std::endl;
			fclose(fp);
			return false;
		}
	}
	fclose(fp);
	return true;
}

bool ProcessUpload(SocketSSLEx& sock, std::vector<char>& result)
{
	std::vector<char> data;
	if ( ! sock.RecvPackage(data))
	{
		std::cerr << "Read data for File command failed!" << std::endl;
		return false;
	}

	std::vector<char> filenameData;
	if ( ! sock.RecvPackage(filenameData))
	{
		std::cerr << "Read filename for File command failed!" << std::endl;
		return false;
	}
	std::string filename(filenameData.begin(), filenameData.end());

	size_t fileSize = 0;
	if (sizeof(fileSize) != sock.RecvEx(&fileSize, sizeof(fileSize)))
	{
		std::cerr << "Read file size failed!" << std::endl;
		return false;
	}
	FILE* fp = fopen((transferFolder + "/" + filename).c_str(), "wb");
	if ( ! fp)
	{
		std::cerr << "Create file failed!" << std::endl;
		return false;
	}
	size_t writen = 0;
	char buffer[32 * 1024];
	while (writen < fileSize)
	{
		int toRead = static_cast<int>(fileSize - writen);
		if (toRead > static_cast<int>(sizeof(buffer)))
		{
			toRead = static_cast<int>(sizeof(buffer));
		}
		int n = sock.RecvEx(buffer, toRead);
		if (n < 0)
		{
			std::cerr << "Recv file data failed!" << std::endl;
			fclose(fp);
			return false;
		}
		if (n > 0)
		{
			if (n != static_cast<int>(fwrite(buffer, 1, n, fp)))
			{
				std::cerr << "Write file failed!" << std::endl;
				fclose(fp);
				return false;
			}
			writen += n;
		}
	}
	fclose(fp);
	std::cout << "File (" << fileSize << " bytes) received." << std::endl;

	return RunScript(data, result);
}

bool Process(SSL_CTX* ctx, const char* id, const char* host, unsigned short port)
{
	SocketSSLEx sock;
	if ( ! sock.Create(false))
	{
		std::cerr << "Create socket failed!" << std::endl;
		return false;
	}
	if ( ! sock.Connect(ctx, host, port))
	{
		std::cerr << "Can not connect to " << host << ":" << port << "!" << std::endl;
		sock.Close();
		return false;
	}
	std::cout << "IDS Connected to CMS!" << std::endl;

	if (sizeof(SYMBOL) != sock.SendEx(SYMBOL, sizeof(SYMBOL)))
	{
		std::cerr << "Send symbol to CMS failed!" << std::endl;
		sock.Close();
		return false;
	}
	
	if ( ! sock.SendPackage(id, strlen(id)))
	{
		std::cerr << "Send ID failed!" << std::endl;
		sock.Close();
		return false;
	}
	std::cout << "Send ID '" << id << "' OK!" << std::endl;

	for (;;)
	{
		int n = sock.WaitData(100, true);
		if (n > 0)
		{
			char cmdData[4];
			if (sizeof(cmdData) != sock.RecvEx(cmdData, sizeof(cmdData)))
			{
				std::cerr << "Read cmd failed!" << std::endl;
				break;
			}
			std::string cmd(cmdData, cmdData + 4);

			std::vector<char> result;

			if (cmd == "Send")
			{
                sending_data_tag = 1;
				if ( ! ProcessSend(sock, result))
				{
					break;
				}
			}
			else if (cmd == "File")
			{
                sending_data_tag = 1;
				if ( ! ProcessUpload(sock, result))
				{
					break;
				}
			}
			else if (cmd == "Down")
			{
                sending_data_tag = 1;
				if ( ! ProcessDownload(sock, result))
				{
					break;
				}
                sending_data_tag = 0;
                check_time = time(NULL);
				continue;
			}
			else if (cmd == "Noop")
			{
                check_time = time(NULL);
				continue;
			}
			else
			{
                sending_data_tag = 1;
				std::string text = CreateJSON(1, "未知命令");
				result.insert(result.begin(), text.begin(), text.end());
			}

			if ( ! sock.SendPackage(&result[0], result.size()))
			{
				std::cerr << "Send result failed!" << std::endl;
				break;
			}

            sending_data_tag = 0;
            check_time = time(NULL);
			std::cout << "Send back data (" << result.size() << " bytes)." << std::endl;
			std::cout << std::string(result.begin(), result.end()) << std::endl;
		}
	}

	sock.Close();
	return true;
}

///////////////////////////////////////////////////////////////////////////

void PipeSignalHandler(int)
{
}

void PrintUsage()
{
	std::cout
		<< "Usage: " << std::endl
		<< "ids-messenger <id> <host> <port> <transfer-folder> <script-conf-file>" << std::endl
		<< std::endl
		<< "Note:" << std::endl
		<< "  id               : Identify code for this IDS server." << std::endl
		<< "  host             : CMS ip address." << std::endl
		<< "  port             : CMS port to connect." << std::endl
		<< "  transfer-folder  : Folder for storing files received or to sent." << std::endl
		<< "  script-conf-file : Configure file to set script for commands." << std::endl
		<< std::endl
		<< "Example:" << std::endl
		<< "  ids-messenger test 192.168.1.51 10102 ./ids-data/ ids.conf" << std::endl
		<< std::endl;
}

///////////////////////////////////////////////////////////////////////////
/************************
@Author:xueming.li
@Reason:To sovle net link delay problem. 
        Set timer thread,when recv Noop timeout,exit!
@Date:2011-12-26
************************/
#include <cstdio>
#include <ctime>
#include <sys/time.h>
#include <errno.h>
#include <cstring>
#include <pthread.h>

#define MY_TIMEOUT 10

char* id;
char* host;
unsigned short port;
SSL_CTX* ctx;

void my_timeout(int sig)
{
    if ((sending_data_tag == 0) && (time(NULL) - check_time >= MY_TIMEOUT))
    {
        fprintf(stdout,"Noop Wait timeout,exit!\n");
        exit(1);
    }
}

void* my_timer(void* arg)
{
    while (1)
    {
        sleep (1);
        kill (getpid (), SIGALRM);
    }

    return ((void*)2);
}

void* my_process(void* arg)
{
	SSL_library_init();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(SSLv23_method());
	if ( ! ctx)
	{
		std::cerr << "SSL_CTX_new failed!" << std::endl;
        exit(1);
	}

	if ( ! Process(ctx, id, host, port))
	{
		SSL_CTX_free(ctx);
        exit(1);
	}

	SSL_CTX_free(ctx);

    return ((void*)1);
}


///////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	std::cout << "NetPower IDS-Messenger v1.0.0" << std::endl << std::endl;

	if (argc < 6)
	{
		PrintUsage();
		return 1;
	}

	id = argv[1];
	host = argv[2];
	port = static_cast<unsigned short>(atoi(argv[3]));
	transferFolder = argv[4];
	const char* scriptConfFile = argv[5];

	struct stat statBuf;
	if (stat(transferFolder.c_str(), &statBuf) || ! S_ISDIR(statBuf.st_mode))
	{
		std::cerr << "Transfer folder '" << transferFolder << "' is invalid!" << std::endl;
		return 1;
	}
	if (stat(scriptConfFile, &statBuf) || ! S_ISREG(statBuf.st_mode))
	{
		std::cerr << "Script configure file '" << scriptConfFile << "' is invalid!" << std::endl;
		return 1;
	}
	std::cout
		<< "ID of this IDS        : " << id << std::endl
		<< "CMS host & port       : " << host << ":" << port << std::endl
		<< "Transfer folder       : " << transferFolder << std::endl
		<< "Script configure file : " << scriptConfFile << std::endl
		<< std::endl;
	signal(SIGPIPE, PipeSignalHandler);

	if ( ! LoadRules(scriptConfFile))
	{
		std::cerr << "Load rule file '" << scriptConfFile << "' failed!" << std::endl;
		return 1;
	}

///////////////////////////////////////////////////////////////////////////
    check_time = time(NULL);
    sending_data_tag = 0;

    signal(SIGALRM,my_timeout);

    int err;
    pthread_t tid1;
    pthread_t tid2;

    err = pthread_create(&tid1,NULL,my_process,NULL);
    if (err != 0)
    {
        fprintf(stderr,"can't create my_process: %s\n",strerror(err));
        exit(1);
    }

    err = pthread_create(&tid2,NULL,my_timer,NULL);
    if (err != 0)
    {
        fprintf(stderr,"can't create my_timer: %s\n",strerror(err));
        exit(1);
    }

    sleep(2);

    pthread_join(tid1,NULL);
    pthread_join(tid2,NULL);

	return 0;
}

///////////////////////////////////////////////////////////////////////////

