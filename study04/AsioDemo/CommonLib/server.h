#pragma once

namespace usr::network
{
	using asio::ip::tcp;
	class cli_base
	{
	public:
		virtual ~cli_base() {}
		virtual void deliver(const pkg_message& msg) = 0;
	};
	using cli_base_ptr=std::shared_ptr<cli_base>;
	//using server_callback = std::function<void(const pkg_message &msg, PVOID _client)>;
	class cli_control
	{
	public:
		cli_control() :cli_count_(0)
		{
		}
		void join(cli_base_ptr participant)
		{
			InterlockedIncrement64(&cli_count_);
			participants_.insert(participant);
		}
		void leave(cli_base_ptr participant)
		{
			InterlockedDecrement64(&cli_count_);
			participants_.erase(participant);
		}
		void deliver(const pkg_message& msg)
		{
			if (cli_count_ == 0)
				return;
			for (auto participant : participants_)
				participant->deliver(msg);
		}
	private:
		long long cli_count_;
		std::set<cli_base_ptr> participants_;
	};
	class iocp_session
		:public cli_base,
		public std::enable_shared_from_this<iocp_session>
	{
	public:
		iocp_session(tcp::socket socket, cli_control& room)
			: socket_(std::move(socket)),
			cli_(room),callback_(nullptr)
		{
		}

		void start(client_callback callback)
		{
			callback_ = callback;

			auto ip = socket_.remote_endpoint().address().to_string();
			auto port = socket_.remote_endpoint().port();
			std::cout << "new client::" << ip << " " << port << std::endl;

			cli_.join(shared_from_this());
			do_read_header();
		}
		void deliver(const pkg_message& msg)
		{
			write(msg);
		}
	private:
		void write(const pkg_message &msg_)
		{
			bool write_in_progress = !write_msgs_.empty();
			write_msgs_.push_back(msg_);
			if (!write_in_progress)
			{
				do_write();
			}
		}
	private:
		void do_read_header()
		{
			auto self(shared_from_this());
			asio::async_read(socket_,
				asio::buffer(read_msg_.data(), pkg_message::header_length),
				[this, self](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec && read_msg_.decode_header())
				{
					do_read_body();
				}
				else
				{
					//头部出现异常 关闭socket
					//cli_.leave(shared_from_this());
					do_shutdown();
				}
			});
		}
		void do_shutdown()
		{
			cli_.leave(shared_from_this());
			asio::error_code ignored_ec;
			socket_.shutdown(asio::ip::tcp::socket::shutdown_both,
				ignored_ec);
		}
		void do_read_body()
		{
			auto self(shared_from_this());
			asio::async_read(socket_,
				asio::buffer(read_msg_.body(), read_msg_.body_length()),
				[this, self](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					//这里应该处理read_msg的内容
					if (read_msg_.decode_header())
						do_cmd_read(read_msg_);
					do_read_header();
				}
				else
				{
					//	cli_.leave(shared_from_this());
					do_shutdown();
				}
			});
		}
		void do_cmd_read(const pkg_message &msg)
		{
			if (callback_)
			{
				callback_(msg, this);
			}
			else
			{
				//获取client信息？
				std::cout << "from client=";
				std::cout.write(msg.body(), msg.body_length());
				cli_.deliver(msg);
			}
		}
		
		void do_write()
		{
			auto self(shared_from_this());
			asio::async_write(socket_,
				asio::buffer(write_msgs_.front().data(),
					write_msgs_.front().length()),
				[this, self](std::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					write_msgs_.pop_front();
					if (!write_msgs_.empty())
					{
						do_write();
					}
					else
					{
						
					}
				}
				else
				{
					//	cli_.leave(shared_from_this());
					do_shutdown();
				}
			});
		}
	private:
		tcp::socket socket_;
		pkg_message read_msg_;
		pkg_message_queue write_msgs_;
		client_callback callback_;
		cli_control &cli_;
	};
	class iocp_server
	{
	public:
		iocp_server(asio::io_service& io_service,
			const tcp::endpoint& endpoint,
			client_callback callback)
			: acceptor_(io_service, endpoint),
			socket_(io_service),my_service(io_service),callback_(callback)
		{
			do_accept();
			_run = std::thread([&]() {my_service.run(); });
		}
		~iocp_server()
		{
			my_service.stop();
			_run.join();
		}

		static iocp_server * init_server(asio::io_service &my,std::string port,client_callback pfn=nullptr)
		{
			try
			{
				tcp::endpoint endpoint(tcp::v4(), std::atoi(port.c_str()));
				return new iocp_server(my, endpoint,pfn);
			}
			catch (...)
			{

			}
			return nullptr;
		}

		void set_callback(client_callback callback)
		{
			callback_ = callback;
		}
	private:
		asio::io_service &my_service;
		std::thread _run;
	private:
		tcp::endpoint endpoint_;
		tcp::socket socket_;
		tcp::acceptor acceptor_;
	private:
		void do_accept()
		{
			acceptor_.async_accept(socket_,
				[this](std::error_code ec)
			{
				if (!ec)
				{
					std::make_shared<iocp_session>(std::move(socket_), control_)->start(callback_);
				}
				do_accept();
			});
		}
		cli_control control_;
		client_callback callback_;
	};
}