#pragma once
namespace ddk::tdi
{
#include "../socket/ksocket/ksocket.h"
	//client
	//server
	class client
	{
	private:
		INT_PTR m_sock;
	public:
		client() :m_sock(0)
		{

		}
		client(INT_PTR Socket)
		{
			if (Socket != 0 && Socket != -1)
				m_sock = Socket;
		}
		~client()
		{
			if (m_sock != 0 && m_sock != -1)
			{
				close(m_sock);
				m_sock = 0;
			}
		}
		client & operator = (client & client)
		{
			this->m_sock = client.get_sock();
			client.clear_sock();
			return *this;
		}
		INT_PTR get_sock()
		{
			return m_sock;
		}
		void clear_sock()
		{
			m_sock = 0;
		}
		BOOL Connect(LPCSTR lpIp, UINT nPort)
		{
			if (m_sock != 0 && m_sock != -1)
			{
				return FALSE;
			}
			struct sockaddr_in  toAddr;
			int status = 0;
			m_sock = socket(AF_INET, SOCK_STREAM, 0);
			if (m_sock == -1)
			{
				LOG_DEBUG("client: socket() error\r\n");
				return FALSE;
			}
			toAddr.sin_family = AF_INET;
			toAddr.sin_port = htons(USHORT(nPort));//7792¶Ë¿Ú
			toAddr.sin_addr.s_addr = inet_addr(lpIp);

			status = connect(m_sock, (struct sockaddr*)&toAddr, sizeof(toAddr));
			if (status < 0)
			{
				LOG_DEBUG("client failed\r\n");
				close(m_sock);
				m_sock = 0;
				return FALSE;
			}

			return TRUE;
		}
		int Send(LPVOID Buffer, UINT nBufferSize)
		{
			if (m_sock == 0 || m_sock == -1)
			{
				return 0;
			}
			int status = send(m_sock, (const char *)Buffer, nBufferSize, 0);
			if (status < 0)
			{
				LOG_DEBUG("failed to send\r\n");
				return 0;
			}
			return status;
		}
		int Recv(PVOID outBuffer, UINT nSize)
		{
			if (m_sock == 0 || m_sock == -1)
			{
				return 0;
			}
			int st = recv(m_sock, (char *)outBuffer, nSize, 0);
			if (st < 0)
			{
				return 0;
			}
			return st;
		}
		void Clear()
		{
			if (m_sock&&m_sock != -1)
			{
				close(m_sock);
				m_sock = 0;
			}
		}
	};
	class server
	{
	private:
		INT_PTR m_sock_srv;
	public:
		using _callback_accept = std::function<void(INT_PTR)>;
		server() :m_sock_srv(0)
		{
			pfn_callback_on_accept = nullptr;
		}
		~server()
		{
			if (m_sock_srv != 0 && m_sock_srv != -1)
			{
				close(m_sock_srv);
			}
		}
		BOOL Bind(USHORT nPort)
		{
			if (m_sock_srv&&m_sock_srv != -1)
			{
				return FALSE;
			}
			struct sockaddr_in  localAddr;
			int  rVal;

			m_sock_srv = socket(AF_INET, SOCK_STREAM, 0);

			if (m_sock_srv == -1)
			{
				LOG_DEBUG("server: socket() error\n");
				return FALSE;
			}

			localAddr.sin_family = AF_INET;
			localAddr.sin_port = htons(nPort);//¶Ë¿Ú7792
			localAddr.sin_addr.s_addr = INADDR_ANY;

			rVal = bind(m_sock_srv, (struct sockaddr*) &localAddr, sizeof(localAddr));

			if (rVal < 0)
			{
				LOG_DEBUG("server: bind error %#x\r\n", rVal);
				close(m_sock_srv);
				m_sock_srv = 0;
				return FALSE;
			}

			rVal = listen(m_sock_srv, SOMAXCONN);

			if (rVal < 0)
			{
				LOG_DEBUG("server: listen error %#x\r\n", rVal);
				close(m_sock_srv);
				m_sock_srv = 0;
				return FALSE;
			}
			return TRUE;
		}
		BOOL Accept(INT_PTR * Socket)
		{
			INT_PTR             reqSocket;
			struct sockaddr_in  remoteAddr;
			int                 remoteLen = sizeof(remoteAddr);
			char                *addrStr;

			reqSocket = accept(m_sock_srv, (struct sockaddr*) &remoteAddr, &remoteLen);

			if (reqSocket != -1)
			{
#if 1
				addrStr = inet_ntoa(remoteAddr.sin_addr);
				if (addrStr)
				{
					LOG_DEBUG("server: connection from %s:%u\r\n", addrStr, ntohs(remoteAddr.sin_port));
					ExFreePool(addrStr);
				}
#endif
				if (Socket)
				{
					*Socket = reqSocket;
				}
				return TRUE;
			}
			return FALSE;
		}
		void StartWorker(_callback_accept onAccept)
		{
			pfn_callback_on_accept = onAccept;
			if (m_sock_srv != 0 && m_sock_srv != -1)
			{
				auto work_thread = ddk::thread(std::bind(&ddk::tdi::server::worker_thread, this));
				work_thread.detach();
			}
		}
		void worker_thread()
		{
			while (1)
			{
				INT_PTR sock = 0;
				if (Accept(&sock))
				{
					pfn_callback_on_accept(sock);
				}
				else
				{
					break;
				}
			}
		}
	private:
		_callback_accept pfn_callback_on_accept;
	protected:
		server & operator = (server &) = delete;
	};
};