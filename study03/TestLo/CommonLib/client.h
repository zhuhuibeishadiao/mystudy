#pragma once
namespace usr::network
{
	using asio::ip::tcp;
	using pkg_message_queue=std::deque<pkg_message> ;
	using client_callback = std::function<void(const pkg_message msg,PVOID _client)>;
	class client
	{
	public:
		client(asio::io_service& io_service,
			tcp::resolver::iterator endpoint_iterator,
			client_callback callback)
			: my_service(io_service),
			socket_(io_service),
			callback_(callback)
		{
			do_connect(endpoint_iterator);
			_call = std::thread([&]() { my_service.run(); });
		}

		void write(const pkg_message& msg)
		{
			my_service.post(
				[this, msg]()
			{
				bool write_in_progress = !write_msgs_.empty();
				write_msgs_.push_back(msg);
				if (!write_in_progress)
				{
					do_write();
				}
			});
		}

		void close()
		{
			my_service.post([this]() { socket_.close(); });
		}
		
		void set_callback(client_callback _callback)
		{
			callback_ = _callback;
		}
	private:
		void do_connect(tcp::resolver::iterator endpoint_iterator)
		{
			asio::async_connect(socket_, endpoint_iterator,
				[this](std::error_code ec, tcp::resolver::iterator)
			{
				if (!ec)
				{
					do_read_header();
				}
			});
		}

		void do_read_header()
		{
			asio::async_read(socket_,
				asio::buffer(read_msg_.data(), pkg_message::header_length),
				[this](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec && read_msg_.decode_header())
				{
					do_read_body();
				}
				else
				{
					socket_.close();
				}
			});
		}

		void do_read_body()
		{
			asio::async_read(socket_,
				asio::buffer(read_msg_.body(), read_msg_.body_length()),
				[this](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					//»Øµ÷
					if (callback_)
					{
						callback_(read_msg_,this);
					}
					else
					{
						std::cout << "from srv=";
						std::cout.write(read_msg_.body(), read_msg_.body_length());
						std::cout << "\n";
					}
					do_read_header();
				}
				else
				{
					socket_.close();
				}
			});
		}

		void do_write()
		{
			asio::async_write(socket_,
				asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				[this](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					write_msgs_.pop_front();
					if (!write_msgs_.empty())
					{
						do_write();
					}
				}
				else
				{
					socket_.close();
				}
			});
		}

	private:
		tcp::socket socket_;
		pkg_message read_msg_;
		pkg_message_queue write_msgs_;
		client_callback callback_;
		asio::io_service &my_service;
		std::thread _call;
	public:
		static client* init_client(asio::io_service &my,std::string ip, std::string port,client_callback fp=nullptr)
		{
			try
			{
				tcp::resolver resolver(my);
				auto endpoints = resolver.resolve({ ip, port });
				return new client(my, endpoints, fp);
			}
			catch (std::exception& e)
			{
				std::cerr << "Exception: " << e.what() << "\n";
			}
			return nullptr;
		}
		~client()
		{
			try
			{
				my_service.stop();
			}
			catch (...)
			{
				
			}
			_call.join();
		}
	};
};