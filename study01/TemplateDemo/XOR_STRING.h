#pragma once
//XOR_STRING使IDA看不到字符串
namespace detail
{
	template<std::size_t index>
	struct encryptor
	{
		static constexpr void encrypt(char *dest, const char *str, char key)
		{
			dest[index] = str[index] ^ key;

			encryptor<index - 1>::encrypt(dest, str, key);
		}
	};

	template<>
	struct encryptor<0>
	{
		static constexpr void encrypt(char *dest, const char *str, char key)
		{
			dest[0] = str[0] ^ key;
		}
	};

	template<std::size_t index>
	struct encryptorw
	{
		static constexpr void encrypt(wchar_t *dest, const wchar_t *str, wchar_t key)
		{
			dest[index] = str[index] ^ key;

			encryptorw<index - 1>::encrypt(dest, str, key);
		}
	};

	template<>
	struct encryptorw<0>
	{
		static constexpr void encrypt(wchar_t *dest, const wchar_t *str, wchar_t key)
		{
			dest[0] = str[0] ^ key;
		}
	};
};

class cryptor
{
public:

	template<std::size_t S>
	class string_encryptor
	{
	public:

		constexpr string_encryptor(const char str[S], char key) :
			_buffer{}, _decrypted{ false }, _key{ key }, _bufferw{}, _keyw{}
		{
			detail::encryptor<S - 1>::encrypt(_buffer, str, key);
		}
		constexpr string_encryptor(const wchar_t str[S], wchar_t key) :
			_bufferw{}, _decrypted{ false }, _keyw{ key }, _key{ 0 }, _buffer{}
		{
			detail::encryptorw<S - 1>::encrypt(_bufferw, str, _keyw);
		}
		const char *decrypt(void) const
		{
			if (_decrypted)
				return _buffer;

			for (auto &c : _buffer)
				c ^= _key;

			_decrypted = true;

			return _buffer;
		}
		const wchar_t *decryptw(void) const
		{
			if (_decrypted)
				return _bufferw;

			for (auto &c : _bufferw)
				c ^= _keyw;

			_decrypted = true;

			return _bufferw;
		}
	private:

		mutable char _buffer[S];
		mutable wchar_t _bufferw[S];
		mutable bool _decrypted;
		const char _key;
		const wchar_t _keyw;
	};

	template<std::size_t S>
	static constexpr auto create(const char(&str)[S])
	{
		return string_encryptor<S>{ str, S };
	}
	template<std::size_t S>
	static constexpr auto create(const wchar_t(&str)[S])
	{
		return string_encryptor<S>{ str, S };
	}
};

#define XOR_STRING_W(X) cryptor::create(X).decryptw()

#define XOR_STRING_A(X) cryptor::create(X).decrypt()