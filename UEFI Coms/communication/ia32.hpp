#pragma once 
#ifndef nt_success
#define nt_success(status) (((long)(status)) >= 0)
#endif

extern "C" NTSTATUS __stdcall RtlAdjustPrivilege( ULONG Privilege,BOOLEAN Enable,BOOLEAN CurrentThread,PBOOLEAN Enabled );
extern "C" NTSTATUS __stdcall NtQuerySystemEnvironmentValueEx( __in PUNICODE_STRING VariableName,__in LPGUID VendorGuid,__out_bcount_opt( *ValueLength ) PVOID Value,__inout PULONG ValueLength,__out_opt PULONG Attributes );

constexpr auto SystemModuleInformation = 11;
constexpr auto SystemHandleInformation = 16;
constexpr auto SystemMirrorMemoryInformation = 30;
constexpr auto SystemExtendedHandleInformation = 64;

constexpr auto mm_copy_memory_physical = 0x1;
constexpr auto mm_copy_memory_virtual = 0x2;

constexpr auto page_size = 0x1000;

typedef enum _POOL_TYPE {
	NonPagedPool,
	NonPagedPoolExecute = NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed = NonPagedPool + 2,
	DontUseThisType,
	NonPagedPoolCacheAligned = NonPagedPool + 4,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS = NonPagedPool + 6,
	MaxPoolType,
	NonPagedPoolBase = 0,
	NonPagedPoolBaseMustSucceed = NonPagedPoolBase + 2,
	NonPagedPoolBaseCacheAligned = NonPagedPoolBase + 4,
	NonPagedPoolBaseCacheAlignedMustS = NonPagedPoolBase + 6,
	NonPagedPoolSession = 32,
	PagedPoolSession = NonPagedPoolSession + 1,
	NonPagedPoolMustSucceedSession = PagedPoolSession + 1,
	DontUseThisTypeSession = NonPagedPoolMustSucceedSession + 1,
	NonPagedPoolCacheAlignedSession = DontUseThisTypeSession + 1,
	PagedPoolCacheAlignedSession = NonPagedPoolCacheAlignedSession + 1,
	NonPagedPoolCacheAlignedMustSSession = PagedPoolCacheAlignedSession + 1,
	NonPagedPoolNx = 512,
	NonPagedPoolNxCacheAligned = NonPagedPoolNx + 4,
	NonPagedPoolSessionNx = NonPagedPoolNx + 32,

} POOL_TYPE;

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[ 256 ];
} RTL_PROCESS_MODULE_INFORMATION,* PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[ 1 ];
} RTL_PROCESS_MODULES,* PRTL_PROCESS_MODULES;

typedef LARGE_INTEGER PHYSICAL_ADDRESS,* PPHYSICAL_ADDRESS;

struct SYSTEM_HANDLE {
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
};

struct SYSTEM_HANDLE_INFORMATION {
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[ 0 ];
};

typedef struct _PHYSICAL_MEMORY_RANGE {
	PHYSICAL_ADDRESS BaseAddress;
	LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE,* PPHYSICAL_MEMORY_RANGE;

// temp 
typedef struct _MI_ACTIVE_PFN {
	union {
		struct {
			struct /* bitfield */
			{
				/* 0x0000 */ unsigned __int64 Tradable : 1; /* bit position: 0 */
				/* 0x0000 */ unsigned __int64 NonPagedBuddy : 43; /* bit position: 1 */
			}; /* bitfield */
		} /* size: 0x0008 */ Leaf;
		struct {
			struct /* bitfield */
			{
				/* 0x0000 */ unsigned __int64 Tradable : 1; /* bit position: 0 */
				/* 0x0000 */ unsigned __int64 WsleAge : 3; /* bit position: 1 */
				/* 0x0000 */ unsigned __int64 OldestWsleLeafEntries : 10; /* bit position: 4 */
				/* 0x0000 */ unsigned __int64 OldestWsleLeafAge : 3; /* bit position: 14 */
				/* 0x0000 */ unsigned __int64 NonPagedBuddy : 43; /* bit position: 17 */
			}; /* bitfield */
		} /* size: 0x0008 */ PageTable;
		/* 0x0000 */ unsigned __int64 EntireActiveField;
	}; /* size: 0x0008 */
} MI_ACTIVE_PFN,* PMI_ACTIVE_PFN; /* size: 0x0008 */

typedef struct _MMPTE_HARDWARE {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Dirty1 : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Owner : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 WriteThrough : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 CacheDisable : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Accessed : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Dirty : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned __int64 LargePage : 1; /* bit position: 7 */
		/* 0x0000 */ unsigned __int64 Global : 1; /* bit position: 8 */
		/* 0x0000 */ unsigned __int64 CopyOnWrite : 1; /* bit position: 9 */
		/* 0x0000 */ unsigned __int64 Unused : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 40; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 ReservedForSoftware : 4; /* bit position: 52 */
		/* 0x0000 */ unsigned __int64 WsleAge : 4; /* bit position: 56 */
		/* 0x0000 */ unsigned __int64 WsleProtection : 3; /* bit position: 60 */
		/* 0x0000 */ unsigned __int64 NoExecute : 1; /* bit position: 63 */
	}; /* bitfield */
} MMPTE_HARDWARE,* PMMPTE_HARDWARE; /* size: 0x0008 */

typedef struct _MMPTE_PROTOTYPE {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 DemandFillProto : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 HiberVerifyConverted : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 ReadOnly : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Combined : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 Unused1 : 4; /* bit position: 12 */
		/* 0x0000 */ __int64 ProtoAddress : 48; /* bit position: 16 */
	}; /* bitfield */
} MMPTE_PROTOTYPE,* PMMPTE_PROTOTYPE; /* size: 0x0008 */

typedef struct _MMPTE_SOFTWARE {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 PageFileReserved : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 PageFileAllocated : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 ColdPage : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 PageFileLow : 4; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 UsedPageTableEntries : 10; /* bit position: 16 */
		/* 0x0000 */ unsigned __int64 ShadowStack : 1; /* bit position: 26 */
		/* 0x0000 */ unsigned __int64 Unused : 5; /* bit position: 27 */
		/* 0x0000 */ unsigned __int64 PageFileHigh : 32; /* bit position: 32 */
	}; /* bitfield */
} MMPTE_SOFTWARE,* PMMPTE_SOFTWARE; /* size: 0x0008 */

typedef struct _MMPTE_TIMESTAMP {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 MustBeZero : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Unused : 3; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 PageFileLow : 4; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 Reserved : 16; /* bit position: 16 */
		/* 0x0000 */ unsigned __int64 GlobalTimeStamp : 32; /* bit position: 32 */
	}; /* bitfield */
} MMPTE_TIMESTAMP,* PMMPTE_TIMESTAMP; /* size: 0x0008 */

typedef struct _MMPTE_TRANSITION {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Write : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 Spare : 1; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 IoTracker : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 PageFrameNumber : 40; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 Unused : 12; /* bit position: 52 */
	}; /* bitfield */
} MMPTE_TRANSITION,* PMMPTE_TRANSITION; /* size: 0x0008 */

typedef struct _MMPTE_SUBSECTION {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 Unused0 : 3; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 ColdPage : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 Unused1 : 3; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 ExecutePrivilege : 1; /* bit position: 15 */
		/* 0x0000 */ __int64 SubsectionAddress : 48; /* bit position: 16 */
	}; /* bitfield */
} MMPTE_SUBSECTION,* PMMPTE_SUBSECTION; /* size: 0x0008 */

typedef struct _MMPTE_LIST {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned __int64 Valid : 1; /* bit position: 0 */
		/* 0x0000 */ unsigned __int64 OneEntry : 1; /* bit position: 1 */
		/* 0x0000 */ unsigned __int64 filler0 : 2; /* bit position: 2 */
		/* 0x0000 */ unsigned __int64 SwizzleBit : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned __int64 Protection : 5; /* bit position: 5 */
		/* 0x0000 */ unsigned __int64 Prototype : 1; /* bit position: 10 */
		/* 0x0000 */ unsigned __int64 Transition : 1; /* bit position: 11 */
		/* 0x0000 */ unsigned __int64 filler1 : 16; /* bit position: 12 */
		/* 0x0000 */ unsigned __int64 NextEntry : 36; /* bit position: 28 */
	}; /* bitfield */

} MMPTE_LIST,* PMMPTE_LIST; /* size: 0x0008 */
typedef struct _MMPTE {
	union {
		union {
			/* 0x0000 */ unsigned __int64 Long;
			/* 0x0000 */ volatile unsigned __int64 VolatileLong;
			/* 0x0000 */ struct _MMPTE_HARDWARE Hard;
			/* 0x0000 */ struct _MMPTE_PROTOTYPE Proto;
			/* 0x0000 */ struct _MMPTE_SOFTWARE Soft;
			/* 0x0000 */ struct _MMPTE_TIMESTAMP TimeStamp;
			/* 0x0000 */ struct _MMPTE_TRANSITION Trans;
			/* 0x0000 */ struct _MMPTE_SUBSECTION Subsect;
			/* 0x0000 */ struct _MMPTE_LIST List;
		}; /* size: 0x0008 */
	} /* size: 0x0008 */ u;
} MMPTE,* PMMPTE; /* size: 0x0008 */

typedef struct _MIPFNBLINK {
	union {
		struct /* bitfield */
		{
			/* 0x0000 */ unsigned __int64 Blink : 40; /* bit position: 0 */
			/* 0x0000 */ unsigned __int64 NodeBlinkLow : 19; /* bit position: 40 */
			/* 0x0000 */ unsigned __int64 TbFlushStamp : 3; /* bit position: 59 */
			/* 0x0000 */ unsigned __int64 PageBlinkDeleteBit : 1; /* bit position: 62 */
			/* 0x0000 */ unsigned __int64 PageBlinkLockBit : 1; /* bit position: 63 */
		}; /* bitfield */
		struct /* bitfield */
		{
			/* 0x0000 */ unsigned __int64 ShareCount : 62; /* bit position: 0 */
			/* 0x0000 */ unsigned __int64 PageShareCountDeleteBit : 1; /* bit position: 62 */
			/* 0x0000 */ unsigned __int64 PageShareCountLockBit : 1; /* bit position: 63 */
		}; /* bitfield */
		/* 0x0000 */ unsigned __int64 EntireField;
		/* 0x0000 */ volatile __int64 Lock;
		struct /* bitfield */
		{
			/* 0x0000 */ unsigned __int64 LockNotUsed : 62; /* bit position: 0 */
			/* 0x0000 */ unsigned __int64 DeleteBit : 1; /* bit position: 62 */
			/* 0x0000 */ unsigned __int64 LockBit : 1; /* bit position: 63 */
		}; /* bitfield */
	}; /* size: 0x0008 */
} MIPFNBLINK,* PMIPFNBLINK; /* size: 0x0008 */

typedef struct _MMPFNENTRY1 {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned char PageLocation : 3; /* bit position: 0 */
		/* 0x0000 */ unsigned char WriteInProgress : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned char Modified : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned char ReadInProgress : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned char CacheAttribute : 2; /* bit position: 6 */
	}; /* bitfield */
} MMPFNENTRY1,* PMMPFNENTRY1; /* size: 0x0001 */

typedef struct _MMPFNENTRY3 {
	struct /* bitfield */
	{
		/* 0x0000 */ unsigned char Priority : 3; /* bit position: 0 */
		/* 0x0000 */ unsigned char OnProtectedStandby : 1; /* bit position: 3 */
		/* 0x0000 */ unsigned char InPageError : 1; /* bit position: 4 */
		/* 0x0000 */ unsigned char SystemChargedPage : 1; /* bit position: 5 */
		/* 0x0000 */ unsigned char RemovalRequested : 1; /* bit position: 6 */
		/* 0x0000 */ unsigned char ParityError : 1; /* bit position: 7 */
	}; /* bitfield */
} MMPFNENTRY3,* PMMPFNENTRY3; /* size: 0x0001 */

typedef struct _MI_PFN_ULONG5 {
	union {
		/* 0x0000 */ unsigned long EntireField;
		struct {
			struct /* bitfield */
			{
				/* 0x0000 */ unsigned long NodeBlinkHigh : 21; /* bit position: 0 */
				/* 0x0000 */ unsigned long NodeFlinkMiddle : 11; /* bit position: 21 */
			}; /* bitfield */
		} /* size: 0x0004 */ StandbyList;
		struct {
			/* 0x0000 */ unsigned char ModifiedListBucketIndex : 4; /* bit position: 0 */
		} /* size: 0x0001 */ MappedPageList;
		struct {
			struct /* bitfield */
			{
				/* 0x0000 */ unsigned char AnchorLargePageSize : 2; /* bit position: 0 */
				/* 0x0000 */ unsigned char Spare1 : 6; /* bit position: 2 */
			}; /* bitfield */
			/* 0x0001 */ unsigned char ViewCount;
			/* 0x0002 */ unsigned short Spare2;
		} /* size: 0x0004 */ Active;
	}; /* size: 0x0004 */
} MI_PFN_ULONG5,* PMI_PFN_ULONG5; /* size: 0x0004 */

typedef struct _MMPFN {
	union {
		/* 0x0000 */ struct _LIST_ENTRY ListEntry;
		struct {
			union {
				union {
					/* 0x0000 */ struct _SINGLE_LIST_ENTRY NextSlistPfn;
					/* 0x0000 */ void* Next;
					struct /* bitfield */
					{
						/* 0x0000 */ unsigned __int64 Flink : 40; /* bit position: 0 */
						/* 0x0000 */ unsigned __int64 NodeFlinkLow : 24; /* bit position: 40 */
					}; /* bitfield */
					/* 0x0000 */ struct _MI_ACTIVE_PFN Active;
				}; /* size: 0x0008 */
			} /* size: 0x0008 */ u1;
			union {
				/* 0x0008 */ struct _MMPTE* PteAddress;
				/* 0x0008 */ unsigned __int64 PteLong;
			}; /* size: 0x0008 */
			/* 0x0010 */ struct _MMPTE OriginalPte;
		}; /* size: 0x0018 */
	}; /* size: 0x0018 */
	/* 0x0018 */ struct _MIPFNBLINK u2;
	union {
		union {
			struct {
				/* 0x0020 */ unsigned short ReferenceCount;
				/* 0x0022 */ struct _MMPFNENTRY1 e1;
				/* 0x0023 */ struct _MMPFNENTRY3 e3;
			}; /* size: 0x0004 */
			struct {
				/* 0x0020 */ unsigned short ReferenceCount;
			} /* size: 0x0002 */ e2;
			struct {
				/* 0x0020 */ unsigned long EntireField;
			} /* size: 0x0004 */ e4;
		}; /* size: 0x0004 */
	} /* size: 0x0004 */ u3;
	/* 0x0024 */ struct _MI_PFN_ULONG5 u5;
	union {
		union {
			struct /* bitfield */
			{
				/* 0x0028 */ unsigned __int64 PteFrame : 40; /* bit position: 0 */
				/* 0x0028 */ unsigned __int64 ResidentPage : 1; /* bit position: 40 */
				/* 0x0028 */ unsigned __int64 Unused1 : 1; /* bit position: 41 */
				/* 0x0028 */ unsigned __int64 Unused2 : 1; /* bit position: 42 */
				/* 0x0028 */ unsigned __int64 Partition : 10; /* bit position: 43 */
				/* 0x0028 */ unsigned __int64 FileOnly : 1; /* bit position: 53 */
				/* 0x0028 */ unsigned __int64 PfnExists : 1; /* bit position: 54 */
				/* 0x0028 */ unsigned __int64 NodeFlinkHigh : 5; /* bit position: 55 */
				/* 0x0028 */ unsigned __int64 PageIdentity : 3; /* bit position: 60 */
				/* 0x0028 */ unsigned __int64 PrototypePte : 1; /* bit position: 63 */
			}; /* bitfield */
			/* 0x0028 */ unsigned __int64 EntireField;
		}; /* size: 0x0008 */
	} /* size: 0x0008 */ u4;
} MMPFN,* PMMPFN; /* size: 0x0030 */

typedef union _virt_addr_t {
	std::uintptr_t value;
	struct {
		std::uint64_t offset : 12;
		std::uint64_t pt_index : 9;
		std::uint64_t pd_index : 9;
		std::uint64_t pdpt_index : 9;
		std::uint64_t pml4_index : 9;
		std::uint64_t reserved : 16;
	};
} virt_addr_t,* pvirt_addr_t;
static_assert( sizeof( virt_addr_t ) == sizeof( void* ),"Size mismatch, only 64-bit supported." );

typedef union _pml4e {
	std::uint64_t value;
	struct {
		std::uint64_t present : 1;          // Must be 1, region invalid if 0.
		std::uint64_t writeable : 1;        // If 0, writes not allowed.
		std::uint64_t user_supervisor : 1;   // If 0, user-mode accesses not allowed.
		std::uint64_t PageWriteThrough : 1; // Determines the memory type used to access PDPT.
		std::uint64_t page_cache : 1; // Determines the memory type used to access PDPT.
		std::uint64_t accessed : 1;         // If 0, this entry has not been used for translation.
		std::uint64_t Ignored1 : 1;
		std::uint64_t large_page : 1;         // Must be 0 for PML4E.
		std::uint64_t Ignored2 : 4;
		std::uint64_t pfn : 36; // The page frame number of the PDPT of this PML4E.
		std::uint64_t Reserved : 4;
		std::uint64_t Ignored3 : 11;
		std::uint64_t nx : 1; // If 1, instruction fetches not allowed.
	};
} pml4e,* ppml4e;
static_assert( sizeof( pml4e ) == sizeof( void* ),"Size mismatch, only 64-bit supported." );

typedef union _pdpte {
	std::uint64_t value;
	struct {
		std::uint64_t present : 1;          // Must be 1, region invalid if 0.
		std::uint64_t rw : 1;        // If 0, writes not allowed.
		std::uint64_t user_supervisor : 1;   // If 0, user-mode accesses not allowed.
		std::uint64_t PageWriteThrough : 1; // Determines the memory type used to access PD.
		std::uint64_t page_cache : 1; // Determines the memory type used to access PD.
		std::uint64_t accessed : 1;         // If 0, this entry has not been used for translation.
		std::uint64_t Ignored1 : 1;
		std::uint64_t large_page : 1;         // If 1, this entry maps a 1GB page.
		std::uint64_t Ignored2 : 4;
		std::uint64_t pfn : 36; // The page frame number of the PD of this PDPTE.
		std::uint64_t Reserved : 4;
		std::uint64_t Ignored3 : 11;
		std::uint64_t nx : 1; // If 1, instruction fetches not allowed.
	};
} pdpte,* ppdpte;
static_assert( sizeof( pdpte ) == sizeof( void* ),"Size mismatch, only 64-bit supported." );

typedef union _pde {
	std::uint64_t value;
	struct {
		std::uint64_t present : 1;          // Must be 1, region invalid if 0.
		std::uint64_t rw : 1;        // If 0, writes not allowed.
		std::uint64_t user_supervisor : 1;   // If 0, user-mode accesses not allowed.
		std::uint64_t PageWriteThrough : 1; // Determines the memory type used to access PT.
		std::uint64_t page_cache : 1; // Determines the memory type used to access PT.
		std::uint64_t accessed : 1;         // If 0, this entry has not been used for translation.
		std::uint64_t Ignored1 : 1;
		std::uint64_t large_page : 1; // If 1, this entry maps a 2MB page.
		std::uint64_t Ignored2 : 4;
		std::uint64_t pfn : 36; // The page frame number of the PT of this PDE.
		std::uint64_t Reserved : 4;
		std::uint64_t Ignored3 : 11;
		std::uint64_t nx : 1; // If 1, instruction fetches not allowed.
	};
} pde,* ppde;
static_assert( sizeof( pde ) == sizeof( void* ),"Size mismatch, only 64-bit supported." );

typedef union _pte {
	std::uint64_t value;
	struct {
		std::uint64_t present : 1;          // Must be 1, region invalid if 0.
		std::uint64_t rw : 1;        // If 0, writes not allowed.
		std::uint64_t user_supervisor : 1;   // If 0, user-mode accesses not allowed.
		std::uint64_t PageWriteThrough : 1; // Determines the memory type used to access the memory.
		std::uint64_t page_cache : 1; // Determines the memory type used to access the memory.
		std::uint64_t accessed : 1;         // If 0, this entry has not been used for translation.
		std::uint64_t Dirty : 1;            // If 0, the memory backing this page has not been written to.
		std::uint64_t PageAccessType : 1;   // Determines the memory type used to access the memory.
		std::uint64_t Global : 1;           // If 1 and the PGE bit of CR4 is set, translations are global.
		std::uint64_t Ignored2 : 3;
		std::uint64_t pfn : 36; // The page frame number of the backing physical page.
		std::uint64_t reserved : 4;
		std::uint64_t Ignored3 : 7;
		std::uint64_t ProtectionKey : 4;  // If the PKE bit of CR4 is set, determines the protection key.
		std::uint64_t nx : 1; // If 1, instruction fetches not allowed.
	};
} pte,* ppte;
static_assert( sizeof( pte ) == sizeof( void* ),"Size mismatch, only 64-bit supported." );

struct pt_entries {
	std::pair<ppml4e,pml4e>	pml4;
	std::pair<ppdpte,pdpte>	pdpt;
	std::pair<ppde,pde>		pd;
	std::pair<ppte,pte>		pt;
};