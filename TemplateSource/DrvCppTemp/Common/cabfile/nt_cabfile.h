#pragma once
namespace ddk::cabfile
{

#pragma pack(push, 1)
	struct CFHEADER
	{
		UCHAR	signature[4]; /*inet file signature */
		ULONG	reserved1;     /* reserved */
		ULONG	cbCabinet;    /* size of this cabinet file in bytes */
		ULONG	reserved2;     /* reserved */
		ULONG	coffFiles;	/* offset of the first CFFILE entry */
		ULONG 	reserved3;     /* reserved */
		UCHAR	versionMinor;   /* cabinet file format version, minor */
		UCHAR	versionMajor;   /* cabinet file format version, major */
		USHORT	cFolders;  /* number of CFFOLDER entries in this */
						   /*    cabinet */
		USHORT	cFiles;      /* number of CFFILE entries in this cabinet */
		USHORT	flags;        /* cabinet file option indicators */
		USHORT	setID;       /* must be the same for all cabinets in a */
							 /*    set */
		USHORT	iCabinet;         /* number of this cabinet file in a set */
		USHORT	cbCFHeader;       /* (optional) size of per-cabinet reserved */
								  /*    area */
		UCHAR	cbCFFolder;       /* (optional) size of per-folder reserved */
								  /*    area */
		UCHAR	cbCFData;         /* (optional) size of per-datablock reserved */
								  /*    area */
		UCHAR	abReserve[1];      /* (optional) per-cabinet reserved area */
		UCHAR	szCabinetPrev[1];  /* (optional) name of previous cabinet file */
		UCHAR	szDiskPrev[1];     /* (optional) name of previous disk */
		UCHAR	szCabinetNext[1];  /* (optional) name of next cabinet file */
		UCHAR	szDiskNext[1];     /* (optional) name of next disk */
	};

	struct CFFOLDER
	{
		ULONG	coffCabStart;  /* offset of the first CFDATA block in this folder */
		USHORT 	cCFData;       /* number of CFDATA blocks in this folder */
		USHORT 	typeCompress;  /* compression type indicator */
	};

	struct CFFILE
	{
		ULONG	cbFile;
		ULONG	uoffFolderStart;
		USHORT	iFolder;
		USHORT	data;
		USHORT	time;
		USHORT	attribs;
		char	szName[1];
	};

	struct CFDATA
	{
		ULONG	csum;
		USHORT	cbData;
		USHORT	cbUncomp;
		UCHAR	ab[1];
	};
#pragma pack(pop)
	bool extract(ddk::nt_memfile &_infile, ddk::nt_memfile &_outfile);
}