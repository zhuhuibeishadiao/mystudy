#pragma once
//内存文件
namespace ddk
{
	class nt_memfile
	{
	public:
		nt_memfile() { 
			init();
		};
		~nt_memfile() {
			if (_memblock)
			{
				free(_memblock);
			}
		};
		nt_memfile(size_t _size) {
			init();
			_memblock = malloc(_size);
			if (_memblock)
			{
				RtlZeroMemory(_memblock, _size);
				_file_size = _size;
			}
		};
		nt_memfile(PVOID _buffer, size_t _size){
			init();
			_memblock = malloc(_size);
			if (_memblock)
			{
				RtlZeroMemory(_memblock, _size);
				RtlCopyMemory(_memblock, _buffer, _size);
				_file_size = _size;
			}
		};
		nt_memfile & operator = (nt_memfile &_mmfile){
			this->_file_size = _mmfile._file_size;
			this->_offset = _mmfile._offset;
			this->_memblock = _mmfile._memblock;
			_mmfile.set_rel();
			return (*this);
		};
	protected:
		void set_rel()
		{
			_memblock = nullptr;
		}
	private:
		PVOID _memblock;
		size_t _offset;
		size_t _file_size;
	private:
		void init()
		{
			_memblock = nullptr;
			_offset = _file_size = 0;
		}
	public:
		bool read(PVOID _buffer, size_t _size_to_read, size_t &_readed_size)
		{
			_readed_size = 0;
			if (!_memblock)
			{
				return false;
			}
			auto read_size = _size_to_read;
			auto _end_offset = _offset + _size_to_read;
			if (_end_offset>_file_size)
			{
				read_size = _file_size - _offset;
				if (read_size==0)
				{
					//已经没有可以读的地方
					return false;
				}
			}
			auto p_copy = reinterpret_cast<PVOID>((ULONG_PTR)_memblock + _offset);
			RtlCopyMemory(_buffer, p_copy, read_size);
			_readed_size = read_size;
			_offset += read_size;
			return true;
		}
		bool write(PVOID _buffer, size_t _size_to_write, size_t & _writed_size)
		{
			_writed_size = 0;
			if (!_memblock)
			{
				return false;
			}
			auto write_size = _size_to_write;
			auto end_offset = _offset + _size_to_write;
			if (end_offset>_file_size)
			{
				//写入大于XX,进行扩展分配
				_memblock = realloc(_memblock, end_offset);
				if (!_memblock)
				{
					return false;
				}
				_file_size = end_offset;
			}
			auto p_copy = reinterpret_cast<PVOID>((ULONG_PTR)_memblock + _offset);
			RtlCopyMemory(p_copy, _buffer, write_size);
			_writed_size = write_size;
			_offset += write_size;
			return true;
		}
		bool seek(size_t _newoffset)
		{
			if (_newoffset>_file_size)
			{
				return false;
			}
			if (!_memblock)
			{
				return false;
			}
			_offset = _newoffset;
			return true;
		}
		size_t get_size() {
			return _file_size;
		}
		size_t get_offset() {
			return _offset;
		}
		PVOID get_data() {
			return _memblock;
		}
	};
}