// MessengerServer.cpp

#include <cassert>
#include <iostream>
#include "MessengerServer.h"
#include "../Common.h"

///////////////////////////////////////////////////////////////////////////

MessengerThread::MessengerThread(SSL_CTX* ctx, const IPAddress& ip, SocketSSLEx* s)
	: ctx_(ctx)
	, ip_(ip)
	, sock_(s)
{
}

bool MessengerThread::Start()
{
	assert(sock_ && sock_->IsValid());
	if ( ! Thread::StartThread())
	{
		sock_->Close();
		return false;
	}
	return true;
}

void MessengerThread::Stop()
{
	sock_->Close();
	Thread::WaitForExit();
}

int MessengerThread::Run()
{
	char buffer[8] = { 0 };
	if (sizeof(buffer) != sock_->RecvEx(buffer, sizeof(buffer), 100))
	if (memcmp(buffer, SYMBOL, sizeof(SYMBOL)) != 0)
	{
		std::cerr << "Invalid client!" << std::endl;
		sock_->Close();
		return 1;
	}

	std::vector<char> idBuffer;
	if ( ! sock_->RecvPackage(idBuffer))
	{
		std::cerr << "Recv ID failed!" << std::endl;
		sock_->Close();
		return 1;
	}
	id_ = std::string(idBuffer.begin(), idBuffer.end());
	std::cout << "ID: " << id_ << std::endl;

	lastTick_ = time(NULL);
	while (sock_->IsValid())
	{
		if (time(NULL) - lastTick_ > 5)
		{
			if ( ! CheckAlive()) break;
		}
		else
		{
			Thread::Sleep(1);
		}
	}
	return 0;
}

bool MessengerThread::CheckAlive()
{
	ThreadLockScope scope(lock_);
	if (4 != sock_->SendEx("Noop", 4))
	{
		std::cerr << "Send Noop command failed!" << std::endl;
		sock_->Close();
		return false;
	}
	lastTick_ = time(NULL);
	return true;
}

bool MessengerThread::Process(const std::vector<char>& data, std::vector<char>& result)
{
	ThreadLockScope scope(lock_);
	if (4 != sock_->SendEx("Send", 4))
	{
		std::cerr << "Send command to IDS failed!" << std::endl;
		sock_->Close();
		return false;
	}
	if ( ! sock_->SendPackage(&data[0], data.size()))
	{
		std::cerr << "Send data for Send command failed!" << std::endl;
		sock_->Close();
		return false;
	}
	if ( ! sock_->RecvPackage(result))
	{
		std::cerr << "Recv result for Send command failed!" << std::endl;
		sock_->Close();
		return false;
	}
	lastTick_ = time(NULL);
	return true;
}

bool MessengerThread::Upload(const std::vector<char>& data, const std::string& path,
	const std::string& filename, std::vector<char>& result)
{
	ThreadLockScope scope(lock_);
	FILE* fp = fopen((path + "/" + filename).c_str(), "rb");
	if ( ! fp)
	{
		std::stringstream ss;
		ss << "{\"error\":1,\"message\":\"Can not open file '" << filename << "'!\",\"result\":{}}";
		std::string text = ss.str();
		result.clear();
		result.insert(result.end(), text.begin(), text.end());
		std::cerr << "Can not open file '" << filename << "'!" << std::endl;
		return false;
	}
	if (4 != sock_->SendEx("File", 4))
	{
		std::cerr << "Send command to IDS failed!" << std::endl;
		sock_->Close();
		fclose(fp);
		return false;
	}
	if ( ! sock_->SendPackage(&data[0], data.size()))
	{
		std::cerr << "Send command data to IDS failed!" << std::endl;
		sock_->Close();
		fclose(fp);
		return false;
	}
	if ( ! sock_->SendPackage(filename.c_str(), filename.size()))
	{
		std::cerr << "Send filename failed!" << std::endl;
		sock_->Close();
		fclose(fp);
		return false;
	}
	fseek(fp, 0, SEEK_END);
	size_t fileSize = static_cast<size_t>(ftell(fp));
	fseek(fp, 0, SEEK_SET);
	std::cout << "File size: " << fileSize << std::endl;

	if (sizeof(fileSize) != sock_->SendEx(&fileSize, sizeof(fileSize)))
	{
		std::cerr << "Send file size failed!" << std::endl;
		sock_->Close();
		fclose(fp);
		return false;
	}
	while ( ! feof(fp))
	{
		char buffer[32 * 1024];
		size_t size = fread(buffer, 1, sizeof(buffer), fp);
		if (static_cast<int>(size) != sock_->SendEx(buffer, size))
		{
			std::cerr << "Send file data failed!" << std::endl;
			sock_->Close();
			fclose(fp);
			return false;
		}
	}
	fclose(fp);

	if ( ! sock_->RecvPackage(result))
	{
		std::cerr << "Recv size from IDS failed!" << std::endl;
		sock_->Close();
		return false;
	}
	lastTick_ = time(NULL);
	return true;
}

bool MessengerThread::Download(const std::string& path, const std::string& filename, std::vector<char>& result)
{
	ThreadLockScope scope(lock_);
	AutoDeleteFile autoDeleteFile(path + "/" + filename);
	FILE* fp = fopen((path + "/" + filename).c_str(), "wb");
	if ( ! fp)
	{
		std::stringstream ss;
		ss << "{\"error\":1,\"message\":\"Can not create file '" << filename << "'!\",\"result\":{}}";
		std::string text = ss.str();
		result.clear();
		result.insert(result.end(), text.begin(), text.end());
		std::cerr << "Can not create file '" << filename << "'!" << std::endl;
		return false;
	}
	if (4 != sock_->SendEx("Down", 4))
	{
		std::cerr << "Send 'Down' command failed!" << std::endl;
		fclose(fp);
		sock_->Close();
		return false;
	}
	if ( ! sock_->SendPackage(filename.c_str(), filename.size()))
	{
		std::cerr << "Send filename for 'Down' command failed!" << std::endl;
		fclose(fp);
		sock_->Close();
		return false;
	}
	size_t resultData = 0;
	if (sizeof(resultData) != sock_->RecvEx(&resultData, sizeof(resultData)))
	{
		std::cerr << "Recv result int failed!" << std::endl;
		fclose(fp);
		sock_->Close();
		return false;
	}
	if (resultData != 0)
	{
		std::cerr << "Download file failed! result value:" << resultData << std::endl;
		fclose(fp);
		sock_->Close();
		return false;
	}
	size_t fileSize = 0;
	if (sizeof(fileSize) != sock_->RecvEx(&fileSize, sizeof(fileSize)))
	{
		std::cerr << "Read file size failed!" << std::endl;
		fclose(fp);
		sock_->Close();
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
		int n = sock_->RecvEx(buffer, toRead);
		if (n < 0)
		{
			std::cerr << "Recv file data failed!" << std::endl;
			fclose(fp);
			sock_->Close();
			return false;
		}
		if (n > 0)
		{
			if (n != static_cast<int>(fwrite(buffer, 1, n, fp)))
			{
				std::cerr << "Write file failed!" << std::endl;
				fclose(fp);
				sock_->Close();
				return false;
			}
			writen += n;
		}
	}
	fclose(fp);
	autoDeleteFile.CancelDelete();
	std::cout << "File (" << fileSize << " bytes) received." << std::endl;

	std::stringstream ss;
	ss << "{\"error\":0,\"message\":\"\",\"result\":{\"text\":\"Download file OK!\"}}";
	std::string text = ss.str();
	result.insert(result.begin(), text.begin(), text.end());

	lastTick_ = time(NULL);
	return true;
}

///////////////////////////////////////////////////////////////////////////

MessengerServer::MessengerServer()
	: transferFolder_(".")
{
}

bool MessengerServer::Start(const char* certFile, unsigned short port, const char* transferFolder)
{
	transferFolder_ = transferFolder;

	SSL_library_init();
	SSL_load_error_strings();

	ctx_ = SSL_CTX_new(SSLv23_method());

	if ( ! SSL_CTX_use_certificate_chain_file(ctx_, certFile))
	{
		std::cerr << "Load certificate file failed!" << std::endl;
		SSL_CTX_free(ctx_);
		return false;
	}
	if ( ! SSL_CTX_use_PrivateKey_file(ctx_, certFile, SSL_FILETYPE_PEM))
	{
		std::cerr << "Load private key faile failed!" << std::endl;
		SSL_CTX_free(ctx_);
		return false;
	}

	if ( ! sock_.Create(false))
	{
		std::cerr << "Create socket failed!" << std::endl;
		SSL_CTX_free(ctx_);
		return false;
	}
	int optval = 1;
	if ( ! sock_.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)))
	{
		std::cerr << "SetSockOpt failed!" << std::endl;
		sock_.Close();
		SSL_CTX_free(ctx_);
		return false;
	}
	if ( ! sock_.Bind(port))
	{
		std::cerr << "Bind socket to port " << port << " failed!" << std::endl;
		sock_.Close();
		SSL_CTX_free(ctx_);
		return false;
	}
	if ( ! sock_.Listen())
	{
		std::cerr << "Listen socket failed!" << std::endl;
		sock_.Close();
		SSL_CTX_free(ctx_);
		return false;
	}

	if ( ! Thread::StartThread())
	{
		std::cerr << "Start thread failed!" << std::endl;
		sock_.Close();
		SSL_CTX_free(ctx_);
		return false;
	}
	return true;
}

void MessengerServer::Stop()
{
	sock_.Close();
	SSL_CTX_free(ctx_);
	Thread::WaitForExit();
}

void MessengerServer::RemoveExitedThreads()
{
	ThreadLockScope scope(lock_);
	for (int i = static_cast<int>(threads_.size()) - 1; i >= 0; --i)
	{
		if (( ! threads_[i]->IsRunning()) &&
			(lockedThreads_.find(threads_[i]) == lockedThreads_.end()))
		{
			delete threads_[i];
			delete sockList_[i];
			threads_.erase(threads_.begin() + i);
			sockList_.erase(sockList_.begin() + i);
		}
	}
}

std::vector<IdsInfo> MessengerServer::GetIdsList()
{
	std::vector<IdsInfo> ids;
	ThreadLockScope scope(lock_);
	for (size_t i = 0; i < threads_.size(); ++i)
	{
		if ( ! threads_[i]->IsRunning()) continue;
		ids.push_back(threads_[i]->GetInfo());
	}
	return ids;
}

bool MessengerServer::Process(const std::string& id,
	const std::vector<char>& data, std::vector<char>& result)
{
	MessengerThread* thread = LockThread(id);
	if ( ! thread) return false;
	bool ok = thread->Process(data, result);
	UnlockThread(thread);
	return ok;
}

bool MessengerServer::Upload(const std::string& id,
	const std::vector<char>& data, const std::string& filename, std::vector<char>& result)
{
	MessengerThread* thread = LockThread(id);
	if ( ! thread) return false;
	bool ok = thread->Upload(data, transferFolder_, filename, result);
	UnlockThread(thread);
	return ok;
}

bool MessengerServer::Download(const std::string& id,
	const std::string& filename, std::vector<char>& result)
{
	MessengerThread* thread = LockThread(id);
	if ( ! thread) return false;
	bool ok = thread->Download(transferFolder_, filename, result);
	UnlockThread(thread);
	return ok;
}

MessengerThread* MessengerServer::LockThread(const std::string& id)
{
	ThreadLockScope scope(lock_);
	for (size_t i = 0; i < threads_.size(); ++i)
	{
		if (threads_[i]->IsRunning() && threads_[i]->GetId() == id)
		{
			if (lockedThreads_.find(threads_[i]) != lockedThreads_.end())
			{
				assert(false);
				std::cerr << "Can not lock IDS thread '" << id << "' more than once!" << std::endl;
				return NULL;
			}
			lockedThreads_.insert(threads_[i]);
			return threads_[i];
		}
	}
	std::cerr << "Can not find IDS with ID: '" << id << "'!" << std::endl;
	return false;
}

void MessengerServer::UnlockThread(MessengerThread* thread)
{
	ThreadLockScope scope(lock_);
	lockedThreads_.erase(thread);
}

int MessengerServer::Run()
{
	for (;;)
	{
		SocketSSLEx* s = new SocketSSLEx;
		sockaddr_in name;
		if ( ! sock_.Accept(ctx_, name, *s)) break;
		if ( ! s->IsValid())
		{
			std::cout << "Invalid SSL client from " << IPAddress(name) << "!" << std::endl;
			delete s;
			continue;
		}
		IPAddress ip(name);
		std::cout << "IDS from " << ip << " connected!" << std::endl;

		MessengerThread* thread = new MessengerThread(ctx_, ip, s);
		if ( ! thread->Start())
		{
			delete thread;
			delete s;
		}
		else
		{
			ThreadLockScope scope(lock_);
			threads_.push_back(thread);
			sockList_.push_back(s);
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////

