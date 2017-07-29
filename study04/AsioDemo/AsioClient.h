#pragma once
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
/*           使用举例
	const std::string raw_ip_address = "127.0.0.1";
	const unsigned short port_num = 3333;

	try
	{
	SyncTCPClient client(raw_ip_address, port_num);
	client.connet();

	std::cout << "Sending request to the server..." << std::endl;

	std::string response = client.Foo(12);

	std::cout << "Response received: " << response << std::endl;
	client.close();
	}
	catch (asio::system_error& ec)
	{
	std::cout << "Error occured! Error code = " << ec.code() <<
	". Message: " << ec.what();
	}
*/
class SyncTCPClient
{
	
public:
	SyncTCPClient(const std::string& raw_ip_address, unsigned short port_num)
		:m_ep(asio::ip::address::from_string(raw_ip_address), port_num), m_sock(m_ios)
	{ 
		m_sock.open(m_ep.protocol());
	};
	~SyncTCPClient() {};

	void connet()
	{
		m_sock.connect(m_ep);
	}

	void close()
	{
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both);
		m_sock.close();
	}
	////这一部分是要自己的写的，接收多少怎么，接收
	std::string Foo(unsigned int sec)
	{
		std::string request = "HAHAHA" + std::to_string(sec) + "\n";
		sendRequest(request);
		return receiveResponse();
	}

private:
	void sendRequest(const std::string& request)
	{
		asio::write(m_sock, asio::buffer(request));
	}

	////这一部分是要自己的写的，接收多少怎么，接收
	std::string receiveResponse()
	{
		asio::streambuf buf;
		asio::read_until(m_sock, buf, '\n');
		std::istream input(&buf);
		std::string reponse;
		std::getline(input, reponse);
		return reponse;
	}
private:
	asio::io_service m_ios;
	asio::ip::tcp::endpoint m_ep;
	asio::ip::tcp::socket m_sock;
};

//////////////////////////////////////////////////////////////////////////

class SyncUDPClient {
public:
	SyncUDPClient() :
		m_sock(m_ios) {
		m_sock.open(asio::ip::udp::v4());
	}
	std::string emulateLongComputationOp(
		unsigned int duration_sec,
		const std::string& raw_ip_address,
		unsigned short port_num) {
		std::string request = "EMULATE_LONG_COMP_OP "
			+ std::to_string(duration_sec)
			+ "\n";
		asio::ip::udp::endpoint ep(
			asio::ip::address::from_string(raw_ip_address),
			port_num);
		sendRequest(ep, request);
		return receiveResponse(ep);
	};
private:
	void sendRequest(const asio::ip::udp::endpoint& ep,
		const std::string& request) {
		m_sock.send_to(asio::buffer(request), ep);
	}
	std::string receiveResponse(asio::ip::udp::endpoint& ep) {
		char response[6];
		std::size_t bytes_recieved =
			m_sock.receive_from(asio::buffer(response), ep);
		m_sock.shutdown(asio::ip::udp::socket::shutdown_both);
		return std::string(response, bytes_recieved);
	}
private:
	asio::io_service m_ios;
	asio::ip::udp::socket m_sock;
};

//////////////////////////////////////////////////////////////////////////
class AsyncTCPClient
{
public:
	AsyncTCPClient() { std::cout << "AsyncTCPClient" << std::endl; };
	~AsyncTCPClient() {};

private:

};


