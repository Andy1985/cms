// ControlServer.cpp

#include <cassert>
#include <iostream>
#include <sstream>
#include "ControlServer.h"
#include "../Common.h"

///////////////////////////////////////////////////////////////////////////

ControlThread::ControlThread()
	: processor_(NULL)
{
}

bool ControlThread::Start(SOCKET sock, IProcessor* processor)
{
	sock_.Attach(sock);
	processor_ = processor;
	return Thread::StartThread();
}

void ControlThread::Stop()
{
	sock_.Close();
	return Thread::WaitForExit();
}

int ControlThread::Run()
{
	char buffer[8] = { 0 };
	if (sizeof(buffer) != sock_.RecvEx(buffer, sizeof(buffer), 100))
	{
		std::cerr << "Recv symbol failed!" << std::endl;
		sock_.Close();
		return 0;
	}
	if (memcmp(buffer, SYMBOL, sizeof(SYMBOL)) != 0)
	{
		std::cerr << "Invalid client!" << std::endl;
		sock_.Close();
		return 0;
	}

	char cmdBuffer[4] = { 0 };
	if (sizeof(cmdBuffer) != sock_.RecvEx(cmdBuffer, sizeof(cmdBuffer), 100))
	{
		std::cerr << "Recv cmd failed!" << std::endl;
		sock_.Close();
		return 0;
	}
	std::string cmd(cmdBuffer, cmdBuffer + 4);

	std::vector<char> result;
	if (cmd == "List")
	{
		std::vector<char> data;
		if ( ! sock_.RecvPackage(data))
		{
			std::cerr << "Recv parameters for List failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string s;
		s.insert(s.begin(), data.begin(), data.end());
		std::cout << "List parameters: '" << s << "'" << std::endl;

		std::vector<IdsInfo> ids = processor_->GetIdsList();
		std::stringstream ss;

		if (s == "")
		{
			ss << "{\"error\":0,\"message\":\"\",\"device_count\":" << ids.size() << "}";
		}
		else
		{
			int begin = 0;
			int end = static_cast<int>(ids.size());

			if (s == "all")
			{
				for (int i = begin; i < static_cast<int>(ids.size()) && i <= end; ++i)
				{
					if (i < static_cast<int>(ids.size()))
						ss << ids[i].ip_ << ":" << ids[i].id_<< ",";
					else
						ss << ids[i].ip_<< ":" << ids[i].id_<< std::endl;
				}

			}
			else 
			{
				if (s != "*")
				{
					size_t pos = s.find_first_of('-');
					if (pos == std::string::npos)
					{
						begin = atoi(s.c_str()) - 1;
						end = begin;
					}
					else
					{
						if (pos > 0)
						{
							begin = atoi(s.c_str()) - 1;
						}
						if (pos + 1 < s.size())
						{
							end = atoi(s.substr(pos + 1).c_str()) - 1;
						}
					}
				}
	
		
				ss << "{" << std::endl;
				ss << "\"error\": 0," << std::endl;
				ss << "\"message\": \"\"," << std::endl;
				ss << "\"result\": { \"list\" : [" << std::endl;
				if (begin > end) std::swap(begin, end);
				if (begin < 0) begin = 0;
				if (begin < static_cast<int>(ids.size()))
				{
					for (int i = begin; i < static_cast<int>(ids.size()) && i <= end; ++i)
					{	
						if (i > begin) ss << ",";
						ss << "{";
						ss << "\"id\":\"" << ids[i].id_ << "\",";
						ss << "\"ip\":\"" << ids[i].ip_ << "\"";
						ss << "}" << std::endl;
					}
				}
				ss << "] }" << std::endl;
				ss << "}" << std::endl;
			}
		}
		std::string text = ss.str();
		result.insert(result.begin(), text.begin(), text.end());
	}
	else if (cmd == "Send")
	{
		std::vector<char> idBuffer;
		if ( ! sock_.RecvPackage(idBuffer))
		{
			std::cerr << "Recv ID failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string id(idBuffer.begin(), idBuffer.end());
		std::cout << "ID: " << id << std::endl;

		std::vector<char> data;
		if ( ! sock_.RecvPackage(data))
		{
			std::cerr << "Recv data of 'Send' command failed!" << std::endl;
			sock_.Close();
			return 0;
		}

		processor_->Process(id, data, result);
	}
	else if (cmd == "File")
	{
		std::vector<char> idBuffer;
		if ( ! sock_.RecvPackage(idBuffer))
		{
			std::cerr << "Recv ID failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string id(idBuffer.begin(), idBuffer.end());
		std::cout << "ID: " << id << std::endl;

		std::vector<char> data;
		if ( ! sock_.RecvPackage(data))
		{
			std::cerr << "Recv data of 'File' command failed!" << std::endl;
			sock_.Close();
			return 0;
		}

		std::vector<char> filenameData;
		if ( ! sock_.RecvPackage(filenameData))
		{
			std::cerr << "Recv filename failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string filename(filenameData.begin(), filenameData.end());

		processor_->Upload(id, data, filename, result);
	}
	else if (cmd == "Down")
	{
		std::vector<char> idBuffer;
		if ( ! sock_.RecvPackage(idBuffer))
		{
			std::cerr << "Recv ID failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string id(idBuffer.begin(), idBuffer.end());
		std::cout << "ID: " << id << std::endl;

		std::vector<char> filenameData;
		if ( ! sock_.RecvPackage(filenameData))
		{
			std::cerr << "Recv filename failed!" << std::endl;
			sock_.Close();
			return 0;
		}
		std::string filename(filenameData.begin(), filenameData.end());

		std::cout << "Download file '" << filename << "' from IDS '" << id << "'" << std::endl;

		processor_->Download(id, filename, result);
	}
	else
	{
		std::stringstream ss;
		ss << "{\"error\":1,\"message\":\"Unknown command '" << cmd << "'!\",\"result\":{}}";
		std::string text = ss.str();
		result.insert(result.begin(), text.begin(), text.end());
	}

	std::cout << "Send result (" << result.size() << " bytes) back to web." << std::endl;
	if ( ! sock_.SendPackage(&result[0], result.size()))
	{
		std::cerr << "Send result to Web failed!" << std::endl;
		sock_.Close();
		return 0;
	}
	sock_.Close();

	return 0;
}

///////////////////////////////////////////////////////////////////////////

ControlServer::ControlServer()
	: processor_(NULL)
{
}

bool ControlServer::Start(unsigned short port, IProcessor* processor, const char* ipListFile)
{
	safeIpList_.Load(ipListFile);
	safeIpList_.Dump();

	if ( ! sock_.Create(false))
	{
		std::cerr << "Create socket failed!" << std::endl;
		return false;
	}
	int optval = 1;
	if ( ! sock_.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
	{
		std::cerr << "SetSockOpt failed!" << std::endl;
		sock_.Close();
		return false;
	}
	if ( ! sock_.Bind(port))
	{
		std::cerr << "Bind socket to port " << port << " failed!" << std::endl;
		sock_.Close();
		return false;
	}
	if ( ! sock_.Listen())
	{
		std::cerr << "Listen socket failed!" << std::endl;
		sock_.Close();
		return false;
	}

	assert( processor != NULL );
	processor_ = processor;
	if ( ! Thread::StartThread())
	{
		std::cerr << "Start thread failed!" << std::endl;
		sock_.Close();
		return false;
	}
	return true;
}

void ControlServer::Stop()
{
	sock_.Close();
	Thread::WaitForExit();
}

void ControlServer::RemoveExitedThreads()
{
	ThreadLockScope scope(lockThreads_);
	for (std::list<ControlThread*>::iterator it = threads_.begin(); it != threads_.end(); )
	{
		ControlThread* pThread = *it;
		if ( ! pThread->IsRunning())
		{
			std::list<ControlThread*>::iterator toRemove = it;
			++it;
			threads_.erase(toRemove);
			delete pThread;
		}
		else
		{
			++it;
		}
	}
}

int ControlServer::Run()
{
	for (;;)
	{
		sockaddr_in name;
		SOCKET csock = sock_.Accept(name);
		SocketEx s(csock);
		if ( ! s.IsValid()) break;

		IPAddress ip(name);
		if ( ! safeIpList_.Check(ip))
		{
			std::cout << "Invalid Web from " << ip << std::endl;
			s.Close();
			continue;
		}
	
		{
			ThreadLockScope scope(lockThreads_);
			ControlThread* pThread = new ControlThread;
			if ( ! pThread->Start(csock, processor_))
			{
				s.Close();
				delete pThread;
			}
			else
			{
				s.Detach();
				threads_.push_back(pThread);
			}
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////

