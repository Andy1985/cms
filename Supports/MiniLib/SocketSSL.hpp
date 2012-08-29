// SocketSSL.hpp
#ifndef __SOCKET_SSL_HPP__
#define __SOCKET_SSL_HPP__
///////////////////////////////////////////////////////////////////////////
#include "Socket.hpp"
#include <openssl/ssl.h>
///////////////////////////////////////////////////////////////////////////

class SocketSSL : public Socket
{
private:
	SSL* ssl_;
	BIO* sbio_;
public:
	SocketSSL(SOCKET s = INVALID_SOCKET) : Socket(s), ssl_(NULL), sbio_(NULL) { }
	void Close()
	{
		if (ssl_)
		{
			SSL_free(ssl_);
			ssl_ = NULL;
			sbio_ = NULL;
		}
		Socket::Close();
	}
public:
	void Attach(SOCKET sockfd)
	{
		assert( ! ssl_);
		Socket::Attach(sockfd);
	}
	SOCKET Detach()
	{
		assert( ! ssl_);
		return Socket::Detach();
	}
private:
	bool SSL_Connect(SSL_CTX* ctx)
	{
		assert( ! ssl_);
		ssl_ = SSL_new(ctx);
		sbio_ = BIO_new_socket(sockfd_, BIO_NOCLOSE);
		SSL_set_bio(ssl_, sbio_, sbio_);
		return (SSL_connect(ssl_) > 0);
	}
	bool SSL_Accept(SSL_CTX* ctx)
	{
		assert( ! ssl_);
		ssl_ = SSL_new(ctx);
		sbio_ = BIO_new_socket(sockfd_, BIO_NOCLOSE);
		SSL_set_bio(ssl_, sbio_, sbio_);
		return (SSL_accept(ssl_) > 0);
	}
public:
	bool Connect(SSL_CTX* ctx, const char* host, unsigned short port, int timeout = 0) // in ms
	{
		if ( ! Socket::Connect(host, port, timeout)) return false;
		if ( ! ctx) return true;
		return SSL_Connect(ctx);
	}
	bool Connect(SSL_CTX* ctx, const sockaddr_in& addr, int timeout = 0) // in ms
	{
		if ( ! Socket::Connect(addr, timeout)) return false;
		if ( ! ctx) return true;
		return SSL_Connect(ctx);
	}
	template <class SocketT>
	bool Accept(SSL_CTX* ctx, sockaddr_in& name, SocketT& csock)
	{
		csock.Attach(Socket::Accept(name));
		if (csock.IsValid() && ctx)
		{
			if ( ! csock.SSL_Accept(ctx))
			{
				csock.Close();
				return true; // This means the listening socket is still valid.
			}
		}
		return csock.IsValid();
	}
public:
	int Send(const char* buffer, int size, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		if (ssl_) return SSL_write(ssl_, buffer, size);
		return Socket::Send(buffer, size, flags);
	}
	int Recv(char* buffer, int size, int flags = 0)
	{
		if ( ! IsValid()) return SOCKET_ERROR;
		if (ssl_) return SSL_read(ssl_, buffer, size);
		return Socket::Recv(buffer, size, flags);
	}
	int SendDgram(const char* buffer, int size, sockaddr_in& name, int flags = 0)
	{
		assert( ! ssl_);
		return Socket::SendDgram(buffer, size, name, flags);
	}
	int RecvDgram(char* buffer, int size, sockaddr_in& name, int flags = 0)
	{
		assert( ! ssl_);
		return Socket::RecvDgram(buffer, size, name, flags);
	}
};

typedef SocketExT<SocketSSL> SocketSSLEx;

///////////////////////////////////////////////////////////////////////////
#endif//__SOCKET_SSL_HPP__

