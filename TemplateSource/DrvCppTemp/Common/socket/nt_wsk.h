#pragma once
#include "../socket/wsk/nt_wsk_socket.h"

namespace ddk::wsk
{
	class tcp_client
	{
	public:
		tcp_client()
		{
			_release = false;
			tcpSocket = nullptr;
			WSKStartup();
		}
		tcp_client(PWSK_SOCKET _socket)
		{
			this->tcpSocket = _socket;
			_release = false;
		}
		~tcp_client() {
			if (!_release && tcpSocket)
			{
				shutdown();
				WSKCleanup();
			}
		}
		tcp_client & operator = (tcp_client &_client)
		{
			this->_release = false;
			this->tcpSocket = _client.get_sock();
			_client.release();
			return *this;
		}
		void release() {
			_release = true;
			tcpSocket = nullptr;
		}
		PWSK_SOCKET get_sock() {
			return tcpSocket;
		}
		bool connect(std::wstring host, std::wstring port)
		{
			if (tcpSocket)
			{
				//已经存在socket
				return false;
			}
			auto g_TcpSocket = CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSK_FLAG_CONNECTION_SOCKET);
			if (g_TcpSocket == NULL) {
				return false;
			}
			auto _exit = std::experimental::make_scope_exit([&]() {CloseSocket(g_TcpSocket); });

			IN4ADDR_SETANY(&LocalAddress);
			//必须绑定一个本地地址
			auto status = Bind(g_TcpSocket, (PSOCKADDR)&LocalAddress);
			if (!NT_SUCCESS(status))
			{
				return false;
			}

			UNICODE_STRING hostName;
			UNICODE_STRING portName;
			RtlInitUnicodeString(&hostName, host.c_str());
			RtlInitUnicodeString(&portName, port.c_str());
			status = ResolveName(&hostName, &portName, nullptr, &RemoteAddress);
			if (!NT_SUCCESS(status))
			{
				return false;
			}
			status = Connect(g_TcpSocket, (PSOCKADDR)&RemoteAddress);
			if (!NT_SUCCESS(status))
			{
				return false;
			}
			tcpSocket = g_TcpSocket;
			_exit.release();
			return true;
		}
		bool send(LPCVOID buffer, ULONG size, ULONG &sizeSent)
		{
			ULONG offset = 0;
			sizeSent = 0;
			while (offset < size)
			{
				auto BytesSent = Send(tcpSocket, (PVOID)((ULONG_PTR)buffer + offset), size - offset, WSK_FLAG_NODELAY);
				if (BytesSent == SOCKET_ERROR)
				{
					break;
				}
				offset += BytesSent;
			}
			sizeSent = offset;
			if (sizeSent == size)
			{
				return true;
			}
			return false;
		}
		bool recv(PVOID buffer, ULONG maxSize, ULONG &recvSize)
		{
			auto ret = Receive(tcpSocket, buffer, maxSize, 0);
			if (ret != SOCKET_ERROR)
			{
				recvSize = ret;
				return true;
			}
			return false;
		}
		bool recv_all(PVOID buffer, ULONG maxSize, ULONG &recvSize)
		{
			ULONG offset = 0;
			while (offset < maxSize)
			{
				ULONG _rSize = 0;
				auto ret = recv((PVOID)((ULONG_PTR)buffer + offset), maxSize - offset, _rSize);
				if (!ret)
				{
					break;
				}
				offset += _rSize;
			}
			recvSize = offset;
			if (recvSize == maxSize)
			{
				return true;
			}
			return false;
		}
		void shutdown()
		{
			DisConnect(tcpSocket);
			CloseSocket(tcpSocket);
			release();
		}
	private:
		SOCKADDR_IN 	LocalAddress;
		SOCKADDR_IN		RemoteAddress;
		PWSK_SOCKET		tcpSocket;
		bool _release;
	};
	class udp_client
	{
	public:
		udp_client()
		{
			_release = false;
			udpSocket = nullptr;
			WSKStartup();

		}
		~udp_client()
		{
			if (!_release && udpSocket)
			{
				shutdown();
				WSKCleanup();
			}
		}
		udp_client & operator =(udp_client &_client)
		{
			this->LocalAddress = _client.LocalAddress;
			this->udpSocket = _client.get_sock();
			this->_release = false;
			_client.release();
			return *this;
		}
		PWSK_SOCKET get_sock()
		{
			return udpSocket;
		}
		void release()
		{
			_release = true;
			udpSocket = nullptr;
		}
		void shutdown()
		{
			CloseSocket(udpSocket);
			release();
		}
		udp_client(USHORT port)
		{
			_release = false;
			udpSocket = nullptr;
			WSKStartup();
			create(port);
		}
		bool create(USHORT port)
		{
			if (udpSocket)
			{
				return false;
			}
			auto g_UdpSocket = CreateSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, WSK_FLAG_DATAGRAM_SOCKET);
			if (g_UdpSocket == NULL) {
				return false;
			}
			IN4ADDR_SETANY(&LocalAddress);
			LocalAddress.sin_port = RtlUshortByteSwap(port);
			// Bind Required
			auto status = Bind(g_UdpSocket, (PSOCKADDR)&LocalAddress);
			if (!NT_SUCCESS(status)) {
				CloseSocket(g_UdpSocket);
				return false;
			}
			udpSocket = g_UdpSocket;
			return true;
		}
		bool sendto(
			std::wstring host,
			std::wstring port,
			LPCVOID buffer,
			ULONG buf_size,
			ULONG &sentSize)
		{
			SOCKADDR_IN		RemoteAddress;
			UNICODE_STRING hostName;
			UNICODE_STRING portName;
			RtlInitUnicodeString(&hostName, host.c_str());
			RtlInitUnicodeString(&portName, port.c_str());
			auto status = ResolveName(&hostName, &portName, nullptr, &RemoteAddress);
			if (!NT_SUCCESS(status))
			{
				return false;
			}
			sentSize = SendTo(udpSocket, PVOID(buffer), buf_size, (PSOCKADDR)&RemoteAddress);
			if (sentSize == SOCKET_ERROR)
			{
				return false;
			}
			return true;
		}
		bool recvfrom(PVOID buffer, ULONG maxSize, ULONG &recvSize, PSOCKADDR RemoteAddress)
		{
			recvSize = ReceiveFrom(udpSocket, buffer, maxSize, RemoteAddress, nullptr);
			if (recvSize != SOCKET_ERROR)
			{
				return true;
			}
			return false;
		}
	private:
		SOCKADDR_IN 	LocalAddress;
		PWSK_SOCKET		udpSocket;
		bool _release;
	};
	class server
	{
	public:
		using _callback = std::function<void(PSOCKADDR_IN, PWSK_SOCKET)>;
		server()
		{
			WSKStartup();
			serSocket = nullptr;
			_pfn = nullptr;
		}
		~server() {
			if (serSocket)
			{
				CloseSocket(serSocket);
				serSocket = nullptr;
				WSKCleanup();
			}
		}
		bool bind(USHORT Port)
		{
			if (serSocket)
			{
				//已经存在socket
				return false;
			}
			auto g_TcpSocket = CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, WSK_FLAG_LISTEN_SOCKET);
			if (g_TcpSocket == NULL) {
				return false;
			}
			auto _exit = std::experimental::make_scope_exit([&]() {CloseSocket(g_TcpSocket); });

			SOCKADDR_IN LocalAddress;
			IN4ADDR_SETANY(&LocalAddress);
			LocalAddress.sin_port = RtlUshortByteSwap(Port);
			auto status = Bind(g_TcpSocket, (PSOCKADDR)&LocalAddress);
			if (!NT_SUCCESS(status))
			{
				return false;
			}
			serSocket = g_TcpSocket;
			_exit.release();
			return true;
		}
		void do_worker(_callback pfn)
		{
			if (!serSocket)
			{
				return;
			}
			_pfn = pfn;
			auto thread = ddk::thread(std::bind(&server::workerThread, this));
			thread.detach();
		}
		PWSK_SOCKET Accept()
		{
			SOCKADDR_IN		LocalAddress = { 0 }, RemoteAddress = { 0 };
			auto Socket = ddk::wsk::Accept(serSocket, (PSOCKADDR)&LocalAddress, (PSOCKADDR)&RemoteAddress);
			return Socket;
		}
		void workerThread()
		{
			while (1)
			{
				SOCKADDR_IN		LocalAddress = { 0 }, RemoteAddress = { 0 };
				auto Socket = ddk::wsk::Accept(serSocket, (PSOCKADDR)&LocalAddress, (PSOCKADDR)&RemoteAddress);
				if (!Socket)
				{
					break;
				}
				if (_pfn)
				{
					_pfn(&RemoteAddress, Socket);
				}
			}
		}
	protected:
		server & operator = (server &) = delete;
	private:
		PWSK_SOCKET serSocket;
		_callback _pfn;
	};
}