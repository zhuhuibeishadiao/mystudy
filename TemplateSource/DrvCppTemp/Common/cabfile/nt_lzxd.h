#pragma once
namespace ddk::cabfile
{
	class LzxdIOWrapper {
	public:
		/*
		* feeds data to the decompressor
		* on error, needs to return -1
		* on EOF, return 0
		* else, block until data is available, and return number of bytes.
		*/
		virtual LONG read(void *buf, size_t size) = 0;

		/*
		* receives decompressed data from the decompressor.
		* must read all bytes (size),
		* return -1 otherwise.
		*/
		virtual LONG write(void *buf, size_t size) = 0;

		virtual ~LzxdIOWrapper() {};
	};
	BOOLEAN LzxdDecompress(unsigned window_bits, unsigned reset_interval, unsigned input_buffer_size, ULONG output_length, LzxdIOWrapper *iowrapper);
};