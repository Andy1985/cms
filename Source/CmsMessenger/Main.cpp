// Main.cpp

#include <cstdlib>
#include <csignal>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "MessengerServer.h"
#include "ControlServer.h"

///////////////////////////////////////////////////////////////////////////

volatile bool exitFlag = false;

void PipeSignalHandler(int)
{
}

void ExitSignalHandler(int)
{
	std::cout << "Caught exit signal! Exiting...." << std::endl;
	exitFlag = true;
}

///////////////////////////////////////////////////////////////////////////

void PrintUsage(const char* prog)
{
	std::cout
		<< "Usage:" << std::endl
		<< "  cms-messenger <cert-file> <port-for-web> <port-for-ids> <ip-list-file> <transfer-folder>" << std::endl
		<< std::endl
		<< "Note:" << std::endl
		<< "  cert-file      : Certification file for SSL." << std::endl
		<< "  port-for-web   : TCP port for Web to manage." << std::endl
		<< "  port-for-ids   : TCP port for IDS to connect." << std::endl
		<< "  ip-list-file   : File that contains save ip list." << std::endl
		<< "  transfer-foler : Folder for storing files received or to sent." << std::endl
		<< std::endl
		<< "Example:" << std::endl
		<< "  cms-messenger cms.pem 10101 10102 iplist.txt ./cms-data/" << std::endl
		<< std::endl;
}

int main(int argc, const char** argv)
{
	std::cout << "NetPower CMS-Messenger v1.0.0" << std::endl << std::endl;

	if (argc < 6)
	{
		PrintUsage(argv[0]);
		return 1;
	}
	const char* certFile = argv[1];
	unsigned short portForWeb = static_cast<unsigned short>(atoi(argv[2]));
	unsigned short portForIds = static_cast<unsigned short>(atoi(argv[3]));
	const char* ipListFile = argv[4];
	const char* transferFolder = argv[5];

	if (portForWeb == 0)
	{
		std::cerr << "Invaid port for Web: '" << argv[2] << "'" << std::endl;
		return 1;
	}
	if (portForIds == 0)
	{
		std::cerr << "Invaid port for IDS: '" << argv[3] << "'" << std::endl;
		return 1;
	}
	struct stat statBuf;
	if (stat(certFile, &statBuf) || ! S_ISREG(statBuf.st_mode))
	{
		std::cerr << "Certification file '" << certFile << "' is invalid!" << std::endl;
		return 1;
	}
	if (stat(ipListFile, &statBuf) || ! S_ISREG(statBuf.st_mode))
	{
		std::cerr << "Safe IP list file '" << ipListFile << "' is invalid!" << std::endl;
		return 1;
	}
	if (stat(transferFolder, &statBuf) || ! S_ISDIR(statBuf.st_mode))
	{
		std::cerr << "Transfer folder '" << transferFolder << "' is invalid!" << std::endl;
		return 1;
	}
	std::cout
		<< "Port for Web       : " << portForWeb << std::endl
		<< "Port for IDS       : " << portForIds << std::endl
		<< "Certification file : " << certFile << std::endl
		<< "Safe IP list file  : " << ipListFile << std::endl
		<< "Transfer folder    : " << transferFolder << std::endl
		<< std::endl;

	std::cout << "Starting Messenger Server (port: " << portForIds << ")...." << std::endl;
	MessengerServer messengerServer;
	if ( ! messengerServer.Start(certFile, portForIds, transferFolder))
	{
		std::cerr << "Start Messenger Server failed!" << std::endl;
		return 1;
	}
	std::cout << "Start Messenger Server OK!" << std::endl;

	std::cout << "Starting Control Server (port: " << portForWeb << ")...." << std::endl;
	ControlServer controlServer;
	if ( ! controlServer.Start(portForWeb, &messengerServer, ipListFile))
	{
		std::cerr << "Start Control Server failed!" << std::endl;
		return 1;
	}
	std::cout << "Start Control Server OK!" << std::endl;

	signal(SIGPIPE, PipeSignalHandler);
	signal(SIGINT, ExitSignalHandler);
	signal(SIGTERM, ExitSignalHandler);
	signal(SIGKILL, ExitSignalHandler);
	time_t lastCheckTime = time(NULL);
	while ( ! exitFlag)
	{
		time_t now = time(NULL);
		if (static_cast<int>(now - lastCheckTime) > 5)
		{
			messengerServer.RemoveExitedThreads();
			controlServer.RemoveExitedThreads();
			lastCheckTime = now;
		}
		Thread::Sleep(100);
	}

	std::cout << "Stopping Control Server...." << std::endl;
	controlServer.Stop();
	std::cout << "Stop Control Server OK!" << std::endl;

	std::cout << "Stopping Messenger Server...." << std::endl;
	messengerServer.Stop();
	std::cout << "Stop Messenger Server OK!" << std::endl;
	return 0;
}

///////////////////////////////////////////////////////////////////////////

