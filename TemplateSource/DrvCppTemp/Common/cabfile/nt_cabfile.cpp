#include "stdafx.h"
#include "nt_lzxd.h"

namespace ddk::cabfile
{
	const auto CAB_COMPTYPE_MASK = (0x000f);
	const auto CAB_COMPTYPE_NONE = (0x0000);
	const auto CAB_COMPTYPE_MSZIP = (0x0001);
	const auto CAB_COMPTYPE_QUANTUM = (0x0002);
	const auto CAB_COMPTYPE_LZX = (0x0003);

	class MemFileLzxdIOWrapper : public LzxdIOWrapper {
	public:

		MemFileLzxdIOWrapper(ddk::nt_memfile *Input, ddk::nt_memfile *Output) {
			this->Input = Input;
			this->Output = Output;
		}
		/*
		* feeds data to the decompressor
		* on error, needs to return -1
		* on EOF, return 0
		* else, block until data is available, and return number of bytes.
		*/
		LONG read(void *buf, size_t size) {
			auto canRead = this->Input->get_size() - this->Input->get_offset();
			if (canRead == 0)
				return 0;
			if (canRead > size)
				canRead = size;
			size_t readzie = 0;
			auto Status = this->Input->read(buf, canRead,readzie);
			if (!Status)
				return -1;
			return (LONG)canRead;
		}

		/*
		* receives decompressed data from the decompressor.
		* must write all bytes (size),
		* return -1 otherwise.
		*/
		LONG write(void *buf, size_t size) {
			size_t wtsize = 0;
			auto Status = this->Output->write(buf,size,wtsize);
			if (!Status)
				return -1;
			else
				return (LONG)size;
		}

		~MemFileLzxdIOWrapper() {
		}
	private:
		ddk::nt_memfile *Input;
		ddk::nt_memfile *Output;
	};
	bool LzxdDecompStub(ddk::nt_memfile &Input,ddk::nt_memfile &Output, ULONG OutputLen, USHORT CompType)
	{
		unsigned int window_bits;
		unsigned int input_buf_size;
		MemFileLzxdIOWrapper IO(&Input, &Output);

		window_bits = (CompType >> 8) & 0x1f;
		input_buf_size = 4096;

		LOG_DEBUG("window_bits %d input_buf_size %d CompType %d input len %d output len %d\r\n",
			window_bits, input_buf_size, CompType, Input.get_size(), OutputLen);

		if (!LzxdDecompress(window_bits, 0, input_buf_size, OutputLen, &IO)) {
			LOG_DEBUG("decompress failed\r\b");
			return false;
		}
		return true;
	}

	bool extract(ddk::nt_memfile &_infile, ddk::nt_memfile &_outfile)
	{
		struct CFHEADER Header = {};
		struct CFFOLDER Folder = {};
		struct CFFILE File = {};
		
		do 
		{
			size_t readsize = 0;
			auto bread = _infile.read(&Header, FIELD_OFFSET(CFHEADER, cbCFHeader), readsize);
			if (!bread)
			{
				LOG_DEBUG("Cant read err HEADER\r\n");
				break;
			}
			if (Header.signature[0] != 'M' || Header.signature[1] != 'S' ||
				Header.signature[2] != 'C' || Header.signature[3] != 'F') 
			{
				LOG_DEBUG("invalid cab signature\r\n");
				break;
			}
			if (Header.flags) {
				LOG_DEBUG("cab opt flags not supported\r\n");
				break;
			}
			if (Header.cFolders != 1 || Header.cFiles != 1) {
				//pdb只有一个文件一个目录
				LOG_DEBUG("Unsupported cFolders %u or cFiles %u\r\n", Header.cFolders, Header.cFiles);
				break;
			}
			LOG_DEBUG("flags %x cFolders %u cFiles %u iCabinet %u coffFiles %u\r\n",
				Header.flags, Header.cFolders, Header.cFiles,
				Header.iCabinet, Header.coffFiles);

			readsize = 0;
			bread = _infile.read(&Folder, sizeof(Folder), readsize);
			if (!bread)
			{
				LOG_DEBUG("Cant read err FOLDER\r\n");
				break;
			}
			LOG_DEBUG("coffCabStart %x cCFData %x typeCompress %x\r\n",
				Folder.coffCabStart, Folder.cCFData, Folder.typeCompress);

			if (((Folder.typeCompress & CAB_COMPTYPE_MASK) != CAB_COMPTYPE_LZX)) {
				LOG_DEBUG("unsupported compress type\r\n");
				break;
			}

			readsize = 0;
			bread = _infile.read(&File, sizeof(File), readsize);
			if (!bread)
			{
				LOG_DEBUG("Cant read err FILE\r\n");
				break;
			}

			LOG_DEBUG("cbFile %u uoffFolderStart %x iFolder %x\r\n",
				File.cbFile, File.uoffFolderStart, File.iFolder);

			if (File.uoffFolderStart != 0) {
				LOG_DEBUG("unsuppored uoffFolderStart\r\n");
				break;
			}

			if (!_infile.seek(Folder.coffCabStart))
			{
				LOG_DEBUG("cant seek %x\r\n", Folder.coffCabStart);
				break;
			}

			auto _lzxfile = ddk::nt_memfile(PAGE_SIZE);
			ULONG Datas = 0;
			ULONG OutputLen = 0;
			while (Datas < Folder.cCFData)
			{
				struct CFDATA DataHdr = {};
				auto bs = _infile.read(&DataHdr, FIELD_OFFSET(struct CFDATA, ab),readsize);
				if (!bs)
				{
					break;
				}
				LOG_DEBUG("data csum %x cbData %x cbUncomp %x\r\n",
					DataHdr.csum, DataHdr.cbData, DataHdr.cbUncomp);
				OutputLen += DataHdr.cbUncomp;
				void *DataBuf = malloc(DataHdr.cbData);
				if (!DataBuf) {
					LOG_DEBUG("cant alloc data buf\r\n");
					break;
				}

				bs = _infile.read(DataBuf, DataHdr.cbData,readsize);
				if (!bs) {
					LOG_DEBUG("cant read err\r\n");
					free(DataBuf);
					break;
				}
				size_t writesize = 0;
				bs = _lzxfile.write(DataBuf, DataHdr.cbData,writesize);
				if (!bs) {
					LOG_DEBUG("cant write err\r\n");
					free(DataBuf);
					break;
				}
				free(DataBuf);
				Datas++;
			}

			if (Datas < Folder.cCFData) {
				break;
			}
			_lzxfile.seek(0);

			return LzxdDecompStub(_lzxfile, _outfile, OutputLen, Folder.typeCompress);
		} while (0);

		return false;
	}
}