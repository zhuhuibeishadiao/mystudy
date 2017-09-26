#pragma once
#include "reg_hive.h"
namespace ddk
{
	class nt_hive
	{
	public:
		nt_hive() {
			pMapViewOfFile = nullptr;
			pStartOf = nullptr;
			pRootNamedKey = nullptr;
		};
		nt_hive(std::wstring file_name) {
			pMapViewOfFile = nullptr;
			pStartOf = nullptr;
			pRootNamedKey = nullptr;
			open(file_name);
		};
		~nt_hive() {};
		nt_hive & operator = (nt_hive &_hive) {
			pMapViewOfFile = _hive.pMapViewOfFile;
			pStartOf = _hive.pStartOf;
			pRootNamedKey = _hive.pRootNamedKey;
			this->_file = _hive._file;
			this->_mapfile = _hive._mapfile;
			return (*this);
		};
	public:
		bool open(std::wstring file_name)
		{
			auto _open = _file.open(file_name);
			if (!_open)
			{
				return false;
			}
			auto _open_map = _mapfile.open(_file.get_handle());
			if (!_open_map)
			{
				return false;
			}
			pMapViewOfFile = _mapfile.get_view();
			if (!pMapViewOfFile)
			{
				return false;
			}
			auto pFh = (PHIVE_HEADER)(pMapViewOfFile);
			if ((pFh->tag == 'fger') && (pFh->fileType == 0))
			{
				auto pBh = (PHIVE_BIN_HEADER)((PBYTE)pFh + sizeof(HIVE_HEADER));
				if (pBh->tag == 'nibh')
				{
					pStartOf = (PBYTE)pBh;
					pRootNamedKey = (PHIVE_KEY_NAMED)((PBYTE)pBh + sizeof(HIVE_BIN_HEADER) + pBh->offsetHiveBin);
					auto b_ret = ((pRootNamedKey->tag == 'kn')
						&& (pRootNamedKey->flags & (HIVE_KEY_NAMED_FLAG_ROOT | HIVE_KEY_NAMED_FLAG_LOCKED)));
					if (!b_ret)
					{
						pMapViewOfFile = nullptr;
						return false;
					}
					return true;
				}
			}
			return false;
		}
	private:
		PHIVE_KEY_NAMED searchKeyNamedInList(IN PHIVE_BIN_CELL pHbC, IN LPCWSTR lpSubKey)
		{
			PHIVE_KEY_NAMED pKn, result = NULL;
			PHIVE_LF_LH pLfLh;
			DWORD i;
			wchar_t * buffer;

			switch (pHbC->tag)
			{
			case 'fl':
			case 'hl':
				pLfLh = (PHIVE_LF_LH)pHbC;
				for (i = 0; i < pLfLh->nbElements && !result; i++)
				{
					pKn = (PHIVE_KEY_NAMED)(pStartOf + pLfLh->elements[i].offsetNamedKey);
					if (pKn->tag == 'kn')
					{
						if (pKn->flags & HIVE_KEY_NAMED_FLAG_ASCII_NAME)
							buffer = ddk::string::a2w((char *)pKn->keyName, pKn->szKeyName);
						else {
							buffer = (wchar_t *)malloc(pKn->szKeyName + sizeof(wchar_t));
							if (buffer)
								RtlCopyMemory(buffer, pKn->keyName, pKn->szKeyName);
						}
						if (buffer)
						{
							if (_wcsicmp(lpSubKey, buffer) == 0)
								result = pKn;
							free(buffer);
						}
					}
				}
				break;
			case 'il':
			case 'ir':
			default:
				break;
			}
			return result;
		}
	public:
		bool RegOpenKeyEx(IN HKEY hKey, 
			IN OPTIONAL LPCWSTR lpSubKey, 
			IN DWORD ulOptions, 
			IN ACCESS_MASK samDesired, 
			OUT PHKEY phkResult)
		{
			if (!pMapViewOfFile)
			{
				return false;
			}
			auto pKn = hKey ? (PHIVE_KEY_NAMED)hKey : pRootNamedKey;
			if (pKn->tag == 'kn')
			{
				if (lpSubKey)
				{
					if (pKn->nbSubKeys && (pKn->offsetSubKeys != -1))
					{
						auto pHbC = (PHIVE_BIN_CELL)(pStartOf + pKn->offsetSubKeys);
						auto ptrF = wcschr(lpSubKey, L'\\');
						if (ptrF)
						{
							auto buffer = (wchar_t *)malloc((ptrF - lpSubKey + 1) * sizeof(wchar_t));
							if (buffer)
							{
								RtlCopyMemory(buffer, lpSubKey, (ptrF - lpSubKey) * sizeof(wchar_t));
								*phkResult = (HKEY)searchKeyNamedInList(pHbC, buffer);
								if (*phkResult)
									RegOpenKeyEx(*phkResult, ptrF + 1, ulOptions, samDesired, phkResult);
								free(buffer);
							}
						}
						else *phkResult = (HKEY)searchKeyNamedInList(pHbC, lpSubKey);
					}
				}
				else *phkResult = (HKEY)pKn;
			}
			return (*phkResult != 0);
		}
	private:
		PHIVE_VALUE_KEY searchValueNameInList(IN HKEY hKey, IN OPTIONAL LPCWSTR lpValueName)
		{
			PHIVE_KEY_NAMED pKn;
			PHIVE_VALUE_LIST pVl;
			PHIVE_VALUE_KEY pVk, pFvk = NULL;
			DWORD i;
			wchar_t * buffer;

			pKn = hKey ? (PHIVE_KEY_NAMED)hKey : pRootNamedKey;
			if (pKn->tag == 'kn')
			{
				if (pKn->nbValues && (pKn->offsetValues != -1))
				{
					pVl = (PHIVE_VALUE_LIST)(pStartOf + pKn->offsetValues);
					for (i = 0; i < pKn->nbValues && !pFvk; i++)
					{
						pVk = (PHIVE_VALUE_KEY)(pStartOf + pVl->offsetValue[i]);
						if (pVk->tag == 'kv')
						{
							if (lpValueName)
							{
								if (pVk->szValueName)
								{
									if (pVk->flags & HIVE_VALUE_KEY_FLAG_ASCII_NAME)
										buffer = ddk::string::a2w((char *)pVk->valueName, pVk->szValueName);
									else 
									{
										buffer = (wchar_t *)malloc(pVk->szValueName + sizeof(wchar_t));
										if (buffer)
										{
											RtlCopyMemory(buffer, pVk->valueName, pVk->szValueName);
										}
									}
									if (buffer)
									{
										if (_wcsicmp(lpValueName, buffer) == 0)
											pFvk = pVk;
										free(buffer);
									}
								}
							}
							else if (!pVk->szValueName)
								pFvk = pVk;
						}
					}
				}
			}
			return pFvk;
		}
	public:
		bool RegQueryValueEx(
			IN HKEY hKey,
			IN OPTIONAL LPCWSTR lpValueName,
			IN LPDWORD lpReserved,
			OUT OPTIONAL LPDWORD lpType,
			OUT OPTIONAL LPBYTE lpData,
			IN OUT OPTIONAL LPDWORD lpcbData)
		{
			UNREFERENCED_PARAMETER(lpReserved);
			if (!pMapViewOfFile)
			{
				return false;
			}

			bool status = false;
			auto pFvk = searchValueNameInList(hKey, lpValueName);

			status = (pFvk != NULL);
			if (status)
			{
				auto szData = pFvk->szData & ~0x80000000;
				if (lpType)
					*lpType = pFvk->typeData;

				if (lpcbData)
				{
					if (lpData)
					{
						status = (*lpcbData >= szData);
						if (status)
						{
							auto dataLoc = (pFvk->szData & 0x80000000) ? &pFvk->offsetData : (PVOID) &(((PHIVE_BIN_CELL)(pStartOf + pFvk->offsetData))->data);
							RtlCopyMemory(lpData, dataLoc, szData);
						}
					}
					*lpcbData = szData;
				}
			}
			return status;
		}
	public:
		bool RegSetValueEx(
			IN HKEY hKey,
			IN OPTIONAL LPCWSTR lpValueName,
			IN DWORD Reserved,
			IN DWORD dwType,
			IN OPTIONAL LPCBYTE lpData,
			IN DWORD cbData)
		{
			//Create鍵值和Value時沒辦法搞定的，插入數據比修改難太多了
			UNREFERENCED_PARAMETER(Reserved);
			bool status = false;
			if (!pMapViewOfFile)
			{
				return false;
			}
			auto pFvk = searchValueNameInList(hKey, lpValueName);
			if (pFvk)
			{
				auto flags = pFvk->szData & 0x80000000;
				auto szData = pFvk->szData & ~0x80000000;
				status = (szData >= cbData);
				if (status)
				{
					pFvk->typeData = dwType;
					pFvk->szData = flags | cbData;
					auto dataLoc = (pFvk->szData & 0x80000000) ? &pFvk->offsetData : (PVOID) &(((PHIVE_BIN_CELL)(pStartOf + pFvk->offsetData))->data);
					RtlCopyMemory(dataLoc, lpData, szData);
				}
			}
			_mapfile.flush();
			return status;
		}
	public:
		bool RegEnumKeyEx(
			IN HKEY hKey,
			IN DWORD dwIndex,
			OUT LPWSTR lpName,
			IN OUT LPDWORD lpcName,
			IN LPDWORD lpReserved,
			OUT OPTIONAL LPWSTR lpClass,
			IN OUT OPTIONAL LPDWORD lpcClass,
			OUT OPTIONAL PFILETIME lpftLastWriteTime)
		{
			bool status = false;
			DWORD szInCar;
			PHIVE_KEY_NAMED pKn, pCandidateKn;
			PHIVE_BIN_CELL pHbC;
			PHIVE_LF_LH pLfLh;
			wchar_t * buffer;
			UNREFERENCED_PARAMETER(lpReserved);
			if (!pMapViewOfFile)
			{
				return false;
			}

			pKn = hKey ? (PHIVE_KEY_NAMED)hKey : pRootNamedKey;
			if (pKn->nbSubKeys && (dwIndex < pKn->nbSubKeys) && (pKn->offsetSubKeys != -1))
			{
				pHbC = (PHIVE_BIN_CELL)(pStartOf + pKn->offsetSubKeys);
				switch (pHbC->tag)
				{
				case 'fl':
				case 'hl':
					pLfLh = (PHIVE_LF_LH)pHbC;
					if (pLfLh->nbElements && (dwIndex < pLfLh->nbElements))
					{
						pCandidateKn = (PHIVE_KEY_NAMED)(pStartOf + pLfLh->elements[dwIndex].offsetNamedKey);
						if ((pCandidateKn->tag == 'kn') && lpName && lpcName)
						{
							if (lpftLastWriteTime)
								*lpftLastWriteTime = pKn->lastModification;

							if (pCandidateKn->flags & HIVE_KEY_NAMED_FLAG_ASCII_NAME)
							{
								szInCar = pCandidateKn->szKeyName;
								status = (*lpcName > szInCar);
								if (status)
								{
									buffer = ddk::string::a2w((char *)pCandidateKn->keyName, szInCar);
									if (buffer)
									{
										RtlCopyMemory(lpName, buffer, szInCar * sizeof(wchar_t));
										free(buffer);
									}
								}
							}
							else
							{
								szInCar = pCandidateKn->szKeyName / sizeof(wchar_t);
								status = (*lpcName > szInCar);
								if (status)
									RtlCopyMemory(lpName, pCandidateKn->keyName, pKn->szKeyName);
							}
							if (status)
								lpName[szInCar] = L'\0';
							*lpcName = szInCar;

							if (lpcClass)
							{
								szInCar = pCandidateKn->szClassName / sizeof(wchar_t);
								if (lpClass)
								{
									status = (*lpcClass > szInCar);
									if (status)
									{
										RtlCopyMemory(lpClass, &((PHIVE_BIN_CELL)(pStartOf + pCandidateKn->offsetClassName))->data, pCandidateKn->szClassName);
										lpClass[szInCar] = L'\0';
									}
								}
								*lpcClass = szInCar;
							}
						}
					}
					break;
				case 'il':
				case 'ir':
				default:
					break;
				}
			}
			return status;
		}
	public:
		bool RegEnumValue(
			IN HKEY hKey,
			IN DWORD dwIndex,
			OUT LPWSTR lpValueName,
			IN OUT LPDWORD lpcchValueName,
			IN LPDWORD lpReserved,
			OUT OPTIONAL LPDWORD lpType,
			OUT OPTIONAL LPBYTE lpData,
			OUT OPTIONAL LPDWORD lpcbData)
		{
			UNREFERENCED_PARAMETER(lpReserved);
			if (!pMapViewOfFile)
			{
				return false;
			}
			bool status = false;
			DWORD szBuffer;
			wchar_t * buffer;
			PHIVE_KEY_NAMED pKn;
			PHIVE_VALUE_LIST pVl;
			PHIVE_VALUE_KEY pVk;
			PVOID dataLoc;

			pKn = hKey ? (PHIVE_KEY_NAMED)hKey : pRootNamedKey;
			if (pKn->tag == 'kn')
			{
				if (pKn->nbValues && (dwIndex < pKn->nbValues) && (pKn->offsetValues != -1))
				{
					pVl = (PHIVE_VALUE_LIST)(pStartOf + pKn->offsetValues);
					pVk = (PHIVE_VALUE_KEY)(pStartOf + pVl->offsetValue[dwIndex]);
					if ((pVk->tag == 'kv') && lpValueName && lpcchValueName)
					{
						if (pVk->szValueName)
						{
							if (pVk->flags & HIVE_VALUE_KEY_FLAG_ASCII_NAME)
							{
								szBuffer = pVk->szValueName + 1;
								buffer = ddk::string::a2w((char *)pVk->valueName, pVk->szValueName);
							}
							else
							{
								szBuffer = pVk->szValueName / sizeof(wchar_t) + 1;
								buffer = (wchar_t *)malloc(pVk->szValueName + sizeof(wchar_t));
								if (buffer)
									RtlCopyMemory(buffer, pVk->valueName, pVk->szValueName);
							}

							if (buffer)
							{
								status = (*lpcchValueName >= szBuffer);
								if (status)
								{
									RtlCopyMemory(lpValueName, buffer, szBuffer * sizeof(wchar_t));
									*lpcchValueName = szBuffer - 1;
								}
								free(buffer);
							}
						}
						else if (!pVk->szValueName)
						{
							lpValueName = NULL;
							*lpcchValueName = 0;
						}

						if (status)
						{
							szBuffer = pVk->szData & ~0x80000000;
							if (lpType)
								*lpType = pVk->typeData;

							if (lpcbData)
							{
								if (lpData)
								{
									status = (*lpcbData >= szBuffer);
									if (status)
									{
										dataLoc = (pVk->szData & 0x80000000) ? &pVk->offsetData : (PVOID) &(((PHIVE_BIN_CELL)(pStartOf + pVk->offsetData))->data);
										RtlCopyMemory(lpData, dataLoc, szBuffer);
									}
								}
								*lpcbData = szBuffer;
							}
						}
					}
				}
			}
			return status;
		}
	private:
		LPVOID pMapViewOfFile;
		PBYTE pStartOf;
		PHIVE_KEY_NAMED pRootNamedKey;
	private:
		nt_file _file;
		nt_file_map _mapfile;

	};

}