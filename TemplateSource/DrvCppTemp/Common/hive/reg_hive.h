#pragma once
#define HIVE_KEY_NAMED_FLAG_VOLATILE	0x0001
#define HIVE_KEY_NAMED_FLAG_MOUNT_POINT	0x0002
#define HIVE_KEY_NAMED_FLAG_ROOT		0x0004
#define HIVE_KEY_NAMED_FLAG_LOCKED		0x0008
#define HIVE_KEY_NAMED_FLAG_SYMLINK		0x0010
#define HIVE_KEY_NAMED_FLAG_ASCII_NAME	0x0020

#define HIVE_VALUE_KEY_FLAG_ASCII_NAME	0x0001

typedef struct _HIVE_HEADER
{
	DWORD tag;
	DWORD seqPri;
	DWORD seqSec;
	FILETIME lastModification;
	DWORD versionMajor;
	DWORD versionMinor;
	DWORD fileType;
	DWORD unk0;
	LONG offsetRootKey;
	DWORD szData;
	DWORD unk1;
	BYTE unk2[64];
	BYTE unk3[396];
	DWORD checksum;
	BYTE padding[3584];
} HIVE_HEADER, *PHIVE_HEADER;

typedef struct _HIVE_BIN_HEADER
{
	DWORD tag;
	LONG offsetHiveBin;
	DWORD szHiveBin;
	DWORD unk0;
	DWORD unk1;
	FILETIME timestamp;
	DWORD unk2;
} HIVE_BIN_HEADER, *PHIVE_BIN_HEADER;

typedef struct _HIVE_BIN_CELL
{
	LONG szCell;
	union {
		WORD tag;
		BYTE data[ANYSIZE_ARRAY];
	};
} HIVE_BIN_CELL, *PHIVE_BIN_CELL;

typedef struct _HIVE_KEY_NAMED
{
	LONG szCell;
	WORD tag;
	WORD flags;
	FILETIME lastModification;
	DWORD unk0;
	LONG offsetParentKey;
	DWORD nbSubKeys;
	DWORD nbVolatileSubKeys;
	LONG offsetSubKeys;
	LONG offsetVolatileSubkeys;
	DWORD nbValues;
	LONG offsetValues;
	LONG offsetSecurityKey;
	LONG offsetClassName;
	DWORD szMaxSubKeyName;
	DWORD szMaxSubKeyClassName;
	DWORD szMaxValueName;
	DWORD szMaxValueData;
	DWORD unk1;
	WORD szKeyName;
	WORD szClassName;
	BYTE keyName[ANYSIZE_ARRAY];
} HIVE_KEY_NAMED, *PHIVE_KEY_NAMED;

typedef struct _HIVE_VALUE_KEY
{
	LONG szCell;
	WORD tag;
	WORD szValueName;
	DWORD szData;
	LONG offsetData;
	DWORD typeData;
	WORD flags;
	WORD __align;
	BYTE valueName[ANYSIZE_ARRAY];
} HIVE_VALUE_KEY, *PHIVE_VALUE_KEY;

typedef struct _HIVE_LF_LH_ELEMENT
{
	LONG offsetNamedKey;
	DWORD hash;
} HIVE_LF_LH_ELEMENT, *PHIVE_LF_LH_ELEMENT;

typedef struct _HIVE_LF_LH
{
	LONG szCell;
	WORD tag;
	WORD nbElements;
	HIVE_LF_LH_ELEMENT elements[ANYSIZE_ARRAY];
} HIVE_LF_LH, *PHIVE_LF_LH;

typedef struct _HIVE_VALUE_LIST
{
	LONG szCell;
	LONG offsetValue[ANYSIZE_ARRAY];
} HIVE_VALUE_LIST, *PHIVE_VALUE_LIST;
