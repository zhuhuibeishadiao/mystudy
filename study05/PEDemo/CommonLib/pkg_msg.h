#pragma once
//#include "stdafx.h"

namespace usr::network
{
	class pkg_message
	{
	public:
		enum { header_length = 8 };
		enum { max_body_length = 4096 };

		pkg_message()
			: body_length_(0)
		{
			data_.resize(header_length+8);
		}

		const char* data() const
		{
			return &data_[0];
		}

		char* data()
		{
			return &data_[0];
		}

		std::size_t length() const
		{
			return header_length + body_length_;
		}

		const char* body() const
		{
			return &data_[0] + header_length;
		}

		char* body()
		{
			return &data_[0] + header_length;
		}

		std::size_t body_length() const
		{
			return body_length_;
		}

		void body_length(std::size_t new_length)
		{
			body_length_ = new_length;
			data_.resize(8+body_length_+header_length);
		}

		bool decode_header()
		{
			char header[header_length + 1] = "";
#pragma warning(push)
#pragma warning(disable:4996)
			std::strncat(header, &data_[0], header_length);
#pragma warning(pop)
			body_length_ = std::atoi(header);
			data_.resize(body_length_ + header_length+8);
			return true;
		}

		void encode_header()
		{
			char header[header_length + 1] = "";
#pragma warning(push)
#pragma warning(disable:4996)
			std::sprintf(header, "%8d", static_cast<int>(body_length_));
			std::memcpy(&data_[0], header, header_length);
#pragma warning(pop)
		}

	private:
		std::vector<char> data_;
		std::size_t body_length_;
	};
};