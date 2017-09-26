#pragma once
typedef struct _KDESCRIPTOR
{
	UINT16 Pad[3];
	UINT16 Limit;
	void* Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

typedef union _KIDTENTRY64              // 11 elements, 0x10 bytes (sizeof) 
{
	struct                              // 6 elements, 0x10 bytes (sizeof)  
	{
		/*0x000*/         UINT16       OffsetLow;
		/*0x002*/         UINT16       Selector;
		struct                          // 5 elements, 0x2 bytes (sizeof)   
		{
			/*0x004*/             UINT16       IstIndex : 3;  // 0 BitPosition                    
			/*0x004*/             UINT16       Reserved0 : 5; // 3 BitPosition                    
			/*0x004*/             UINT16       Type : 5;      // 8 BitPosition                    
			/*0x004*/             UINT16       Dpl : 2;       // 13 BitPosition                   
			/*0x004*/             UINT16       Present : 1;   // 15 BitPosition                   
		};
		/*0x006*/         UINT16       OffsetMiddle;
		/*0x008*/         ULONG32      OffsetHigh;
		/*0x00C*/         ULONG32      Reserved1;
	};
	/*0x000*/     UINT64       Alignment;
}KIDTENTRY64, *PKIDTENTRY64;


/// See: Page-Fault Error Code
union PageFaultErrorCode {
	ULONG32 all;
	struct {
		ULONG32 present : 1;   //!< [1] 0= NotPresent
		ULONG32 write : 1;     //!< [2] 0= Read
		ULONG32 user : 1;      //!< [3] 0= CPL==0
		ULONG32 reserved : 1;  //!< [4]
		ULONG32 fetch : 1;     //!< [5]
		ULONG32 reserved1 : 10;
		ULONG32 sgx : 1;
	} fields;
};
static_assert(sizeof(PageFaultErrorCode) == 4, "Size check");

///IA32_EFER MSR Layout
union IA32_EFER_MSR
{
	ULONG64 all;
	struct {
		ULONG64 SCE : 1; //syscall
		ULONG64 reserved0 : 7;
		ULONG64 LME : 1;
		ULONG64 reserved1 : 1;
		ULONG64 LMA : 1;
		ULONG64 NXE : 1;
		ULONG64 reserved2 : 52;
	}fields;
};
static_assert(sizeof(IA32_EFER_MSR) == 8, "Size check");
