#ifndef INTERFACE_CPP
#define INTERFACE_CPP
#include "edk.hpp"
#include <ntstatus.h>
#include <chrono>
#include <TlHelp32.h>
#include <utility>


extern "C" NTSTATUS inline __stdcall RtlAdjustPrivilege(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
extern "C" NTSTATUS  inline __stdcall NtQuerySystemEnvironmentValueEx(__in PUNICODE_STRING VariableName, __in LPGUID VendorGuid, __out_bcount_opt(*ValueLength) PVOID Value, __inout PULONG ValueLength, __out_opt PULONG Attributes);


inline auto uefi_interface::c_interface::get_kernel_image(
	const std::string& module_name ) -> const std::uintptr_t {
	void* buffer = nullptr;
	unsigned long buffer_size = 0;

	auto status = NtQuerySystemInformation(
		static_cast< SYSTEM_INFORMATION_CLASS >( SystemModuleInformation ),
		buffer,
		buffer_size,
		&buffer_size
	);

	while ( status == STATUS_INFO_LENGTH_MISMATCH ) {
		VirtualFree( buffer,NULL,MEM_RELEASE );
		buffer = VirtualAlloc( nullptr,buffer_size,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE );
		status = NtQuerySystemInformation( static_cast< SYSTEM_INFORMATION_CLASS >( SystemModuleInformation ),buffer,buffer_size,&buffer_size );
	}

	if ( !NT_SUCCESS( status ) ) {
		VirtualFree( buffer,NULL,MEM_RELEASE );
		return 0;
	}

	const auto modules = static_cast< PRTL_PROCESS_MODULES >( buffer );
	for ( auto idx = 0u; idx < modules->NumberOfModules; ++idx ) {
		const auto current_module_name = std::string( reinterpret_cast< char* >( modules->Modules[ idx ].FullPathName ) + modules->Modules[ idx ].OffsetToFileName );
		if ( !_stricmp( current_module_name.c_str( ),module_name.c_str( ) ) ) {
			const auto result = reinterpret_cast< uint64_t >( modules->Modules[ idx ].ImageBase );
			VirtualFree( buffer,NULL,MEM_RELEASE );
			return result;
		}
	}

	VirtualFree( buffer,NULL,MEM_RELEASE );
	return 0;
}

auto uefi_interface::c_interface::get_kernel_export(
	const std::uintptr_t image_base,
	const std::string& module_name ) -> const std::uintptr_t {

	/*
	std::unique_ptr< std::remove_pointer_t< HMODULE >,decltype( &FreeLibrary ) > ntoskrnl {
		LoadLibraryA( "ntoskrnl.exe" ),&FreeLibrary
	};*/

	const auto module = LoadLibraryA( "ntoskrnl.exe" );
	if ( !module ) {
		return 0;
	}

	std::uintptr_t address =
		reinterpret_cast< std::uintptr_t >(
			GetProcAddress(
				module,
				module_name.c_str( ) ) );
	if ( !address ) {
		return 0;
	}

	address = address - reinterpret_cast< std::uintptr_t >( module );
	address = address + image_base;

	FreeLibrary( module );
	return address;
}

uefi_interface::c_interface::c_interface( ) {

	bool enabled = false;
	auto status = RtlAdjustPrivilege(
		22L,
		true,
		false,
		reinterpret_cast< BOOLEAN * >( &enabled ) );
	if ( !nt_success( status ) ) {
		printf( "failed to raise perms.\n");
	}

	auto get_import = [&] ( const std::string &image, const std::string &module ) -> std::uintptr_t
	{

		const auto ntoskrnl_base = this->get_kernel_image( image.c_str( ) );
		if ( !ntoskrnl_base ) {
			printf( "failed to get ntoskrnl.\n", true );
			//LOG( OBFUSCATE_STR( "failed to get ntoskrnl.\n" ) );
		}

		const auto func_addr =  this->get_kernel_export(
			ntoskrnl_base,
			module.c_str( )
		);

		return func_addr;
	};

	this->mm_copy_memory = get_import( ( "ntoskrnl.exe" ), ( "MmCopyMemory" ) );
	this->mm_map_space = get_import( ( "ntoskrnl.exe" ), ( "MmMapIoSpace" ) );
	this->mm_unmap_space = get_import( ( "ntoskrnl.exe" ), ( "MmUnmapIoSpace" ) );
	this->mm_physical_ranges = get_import( ( "ntoskrnl.exe" ), ( "MmGetPhysicalMemoryRanges" ) );
	this->ps_initial_system_process = get_import( ( "ntoskrnl.exe" ), ( "PsInitialSystemProcess" ) );
	this->ps_get_process_id = get_import( ( "ntoskrnl.exe" ), ( "PsGetCurrentProcessId" ) );
	this->m_memcpy = get_import( ( "ntoskrnl.exe" ), ( "memcpy" ) );
}

auto uefi_interface::c_interface::attach_vm(
	const std::wstring& proc_name ) -> bool {

	if ( proc_name.empty( ) )
		return false;

	static bool m_fn_found = false;

	int a_pid = 0;

	while (!m_fn_found) {
	a_pid = this->get_process_pid(proc_name);
	printf("e_process : 0x%lx\n", a_pid);

	printf("looking for %s", proc_name);
		if (a_pid)
		{
			printf(("\nfound!\n"));
			m_fn_found = true;
		}
	}

	auto e_process = this->get_eprocess( a_pid );
	printf("e_process : 0x%lx\n", e_process);
	if ( !e_process )
		return false;

	auto base_address = this->get_image_base( e_process );
	if ( !base_address )
		return false;

	printf( ( "base address: %llx\n" ), base_address );

	auto time_now = std::chrono::high_resolution_clock::now( );

	auto pml4e = this->get_directory_table_base( base_address, a_pid );
	if ( !pml4e )
		return false;

	auto time_span = std::chrono::duration_cast< std::chrono::duration< float > >(
		std::chrono::high_resolution_clock::now( ) - time_now
	);

	printf( ( "resolved cr3 in %fms\n" ), time_span.count( ) );
	printf( ( "directory table base: %llx\n" ), pml4e );
	
	this->m_pid = std::move( a_pid );
	this->m_eprocess = std::move( reinterpret_cast< void* >( e_process ) );
	this->m_image_base = std::move( base_address );
	this->m_dtb = std::move( pml4e );

	return true;
}

auto uefi_interface::c_interface::send_cmd(
	std::uintptr_t data,
	void* parameters ) -> bool {

	auto gs_segment = __readgsqword( 0x60 );
	gs_segment = *reinterpret_cast< std::uintptr_t* > ( gs_segment + 0x18 );
	gs_segment = *reinterpret_cast< std::uintptr_t* > ( gs_segment + 0x20 );

	*reinterpret_cast< std::uintptr_t* >( gs_segment + 0x18 ) = data;

	*reinterpret_cast< std::uintptr_t* >( gs_segment + 0x10 ) =
		reinterpret_cast< std::uintptr_t >( parameters );

	UNICODE_STRING string;
	RtlInitUnicodeString( &string, L"uefi_interface" );

	std::uint8_t cmd = 0;
	unsigned long return_length = 1;
	unsigned long attributes = 0;

	const auto status = NtQuerySystemEnvironmentValueEx(
		&string,
		&efi_global_variable_guid,
		&cmd,
		&return_length,
		&attributes );
	if ( nt_success( status ) )
		return false;

	this->cache_rax = *reinterpret_cast< std::uintptr_t* >( gs_segment + 0x18 );
	*reinterpret_cast< std::uintptr_t* >( gs_segment + 0x18 ) = 0;
	*reinterpret_cast< std::uintptr_t* >( gs_segment + 0x10 ) = 0;

	return status;
}

auto uefi_interface::c_interface::invoke_kernel(
	std::uintptr_t address,
	std::uintptr_t a1,
	std::uintptr_t a2,
	std::uintptr_t a3,
	std::uintptr_t a4,
	std::uintptr_t a6 ) -> std::uintptr_t {

	payload arguments{
		.arg1 = a1,
		.arg2 = a2,
		.arg3 = a3,
		.arg4 = a4,
		.arg5 = 0,
		.arg6 = a6, // will be magic code soon just testing for now 
		.arg7 = cmd::invoke
	};

	std::uintptr_t rsp = 0;
	arguments.arg5 = reinterpret_cast< std::uintptr_t >( &rsp );

	auto status = this->send_cmd(
		address,
		&arguments
	);
	if ( !status )
		return 0;

	return *reinterpret_cast< std::uintptr_t* >( &rsp );
}


auto uefi_interface::c_interface::translate_linear(
	pt_entries &entries,
	std::uintptr_t virtual_address,
	std::uintptr_t directory_base ) -> const std::uintptr_t {

	if ( !directory_base || !virtual_address )
		return { };

	// Read PML4E
	auto pml4 = this->read_kernel_physical<std::uintptr_t>( directory_base + 8 * ( ( virtual_address >> 39 ) & 0x1FF ) );
	if ( !pml4 || ( pml4 & 1 ) == 0 ) // Check if present
		return 0;

	// Calculate address of PDPTE and read it
	auto pdpte = this->read_kernel_physical<std::uintptr_t>( ( pml4 & 0xFFFFFFFFFF000 ) + 8 * ( ( virtual_address >> 30 ) & 0x1FF ) );
	if ( !pdpte || ( pdpte & 1 ) == 0 ) // Check if present
		return 0;

	// Check if PDPTE maps a large page
	if ( pdpte & 0x80 )
		return ( virtual_address & 0x3FFFFFFF ) + ( pdpte & 0xFFFFFC00000 );

	// Calculate address of PDE and read it
	auto pd = this->read_kernel_physical<std::uintptr_t>( ( pdpte & 0xFFFFFFFFFF000 ) + 8 * ( ( virtual_address >> 21 ) & 0x1FF ) );
	if ( !pd || ( pd & 1 ) == 0 ) // Check if present
		return 0;

	// Check if PD maps a large page
	if ( pd & 0x80 )
		return ( virtual_address & 0x1FFFFF ) + ( pd & 0xFFFFFE00000 );

	// Calculate address of PTE and read it
	auto pte = this->read_kernel_physical<std::uintptr_t>( ( pd & 0xFFFFFFFFFF000 ) + 8 * ( ( virtual_address >> 12 ) & 0x1FF ) );
	if ( !pte || ( pte & 1 ) == 0 ) // Check if present
		return 0;

	// Return the physical address calculated from the PTE
	return ( virtual_address & 0xFFF ) + ( pte & 0xFFFFFFFFFF000 );
}


auto uefi_interface::c_interface::get_eprocess(
	std::uint32_t process_id ) -> const std::uintptr_t {
	const auto system_eprocess = read_kernel_virtual<std::uintptr_t>( ps_initial_system_process );
	if ( !system_eprocess ) {
		return 0;
	}

	// read active process link
	auto active_process_links = read_kernel_virtual<_LIST_ENTRY>( system_eprocess + 0x448 );
	if ( &active_process_links ) {
		while ( true ) {
			const auto apl_flink = reinterpret_cast< std::uintptr_t >( active_process_links.Flink ) - 0x448;
			if ( apl_flink ) {
				const auto next_process_id = read_kernel_virtual<std::uintptr_t>( apl_flink + 0x440 );
				active_process_links = read_kernel_virtual<_LIST_ENTRY>( apl_flink + 0x448 );

				if ( next_process_id == process_id ) {
					return apl_flink;
				}

				if ( next_process_id == 4 ||
					next_process_id == 0 ) {
					break;
				}
			}
		}
	}
	return 0;
}

auto uefi_interface::c_interface::get_image_base(
	const std::uintptr_t e_process ) -> const std::uintptr_t {
	std::uintptr_t base_address = 0;

	const auto result = read_virtua_km(
		( e_process + 0x520 ),
		reinterpret_cast< std::uint8_t* >( &base_address ),
		sizeof( base_address )
	);
	if ( !result ) {
		return 0;
	}

	return base_address;
}

auto uefi_interface::c_interface::read_physical_km(
	std::uintptr_t address,
	void* buffer,
	std::uintptr_t length ) -> bool {

	auto result = invoke_kernel(
		mm_copy_memory,
		reinterpret_cast< std::uintptr_t >( buffer ),
		address,
		length,
		mm_copy_memory_physical );

	if ( result == length ) {
		return true;
	}

	return false;
}

auto uefi_interface::c_interface::read_virtua_km(
	std::uintptr_t address,
	void* buffer,
	std::uintptr_t length ) -> bool {

	auto result = invoke_kernel(
		this->mm_copy_memory,
		reinterpret_cast< std::uintptr_t >( buffer ),
		address,
		length,
		mm_copy_memory_virtual );

	if ( result == length ) {
		return true;
	}
	return false;
}

auto uefi_interface::c_interface::write_physical_km(
	std::uintptr_t address,
	void* buffer,
	std::uintptr_t length ) -> bool {

	auto allocation = this->invoke< std::uintptr_t >(
		mm_map_space,
		address,
		length,
		PAGE_READWRITE
	);

	this->invoke< void* >(
		m_memcpy,
		allocation,
		reinterpret_cast< std::uintptr_t > ( buffer ),
		length
	);

	this->invoke_kernel(
		this->mm_unmap_space,
		allocation,
		length
	);

	return true;
}
#define min(a,b)            (((a) < (b)) ? (a) : (b))

auto uefi_interface::c_interface::write_physical(
	const std::uintptr_t address,
	void* buffer,
	const std::size_t size ) -> bool {

	if ( address >= 0x7FFFFFFFFFFF ||
		!buffer || !this->m_dtb ) {
		return false;
	}

	pt_entries entries{ };
	auto physical_address = translate_linear(
		entries,
		address,
		this->m_dtb );
	if ( !physical_address ) {
		return false;
	}

	uint64_t final_size = min(
		0x1000 - ( physical_address & 0xFFF ),
		size );

	auto result = write_physical_km(
		physical_address,
		buffer,
		final_size );
	if ( !result ) {
		return false;
	}

	return true;
}

auto uefi_interface::c_interface::read_physical(
	const std::uintptr_t address,
	void* buffer,
	const std::uintptr_t size ) -> bool {

	if ( address >= 0x7FFFFFFFFFFF ||
		!buffer || !this->m_dtb ) {
		return false;
	}

	pt_entries entries{ };
	auto physical_address = translate_linear(
		entries,
		address,
		this->m_dtb );
	if ( !physical_address ) {
		return false;
	}

	auto final_size = min(
		0x1000 - ( physical_address & 0xFFF ),
		size );

	auto result = read_physical_km(
		physical_address,
		buffer,
		final_size );
	if ( !result ) {
		return false;
	}

	return true;
}

auto uefi_interface::c_interface::is_dtb_invalid(
	std::uintptr_t cr3 ) -> bool {
	return ( cr3 >> 0x38 ) == 0x40;
}

auto uefi_interface::c_interface::get_directory_table_base(
	std::uintptr_t image_base,
	int pid ) ->  std::uintptr_t {

	auto e_proc = this->get_eprocess( pid );
	if ( !e_proc ) {
		return false;
	}

	auto current_dtb = this->read_kernel_virtual<std::uintptr_t>( e_proc + 0x28 );
	if ( !current_dtb ) {
		current_dtb = this->read_kernel_virtual<std::uintptr_t>( e_proc + 0x388 );
	}

	return 0x488f1a000;

	virt_addr_t virt_base{
		.value = image_base
	};

	pt_entries entries{ };

	auto ranges = invoke< PHYSICAL_MEMORY_RANGE* >( this->mm_physical_ranges );
	for ( auto i = 0; /**/; i++ ) {

		PHYSICAL_MEMORY_RANGE current_element = { 0 };
		this->invoke< void* >(
			m_memcpy,
			reinterpret_cast< std::uintptr_t >( &current_element ),
			reinterpret_cast< std::uintptr_t >( &ranges[i] ),
			sizeof( PHYSICAL_MEMORY_RANGE )
		);

		if ( !current_element.BaseAddress.QuadPart ||
			!current_element.NumberOfBytes.QuadPart )
			break;

		auto current_physical = current_element.BaseAddress.QuadPart;

		if ( ( current_element.NumberOfBytes.QuadPart / page_size ) < 200  ) {
			continue;
		}

		for ( auto x = 0; x < ( current_element.NumberOfBytes.QuadPart / page_size ); x++,current_physical += page_size ) {
			
			const std::uint64_t pml4_threshold = 30;
			const std::uint64_t pdpte_threshold = 30;
			const std::uint64_t pd_threshold = 20;
			const std::uint64_t pte_hreshold = 10;

			// pml4e
			auto is_pml4e_cached = cache_pml4e.find( current_physical + 8 * virt_base.pml4_index );
			if ( is_pml4e_cached != cache_pml4e.end( ) ) {
				entries.pml4.second = is_pml4e_cached->second;
				entries.pml4.second.accessed++;
			} else {
				entries.pml4.first = reinterpret_cast< ::pml4e* >( current_physical + 8 * virt_base.pml4_index );
				entries.pml4.second = this->read_kernel_physical<::pml4e>( reinterpret_cast< std::uintptr_t >( entries.pml4.first ) );

				if ( entries.pml4.second.value ) {
					cache_pml4e[ current_physical + 8 * virt_base.pml4_index ] = entries.pml4.second;
					entries.pml4.second.accessed = 1;
				}
			}

			if ( is_pml4e_cached != cache_pml4e.end( ) && !entries.pml4.second.value ) {
				cache_pml4e.erase( current_physical + 8 * virt_base.pml4_index );
			}

			if ( is_pml4e_cached != cache_pml4e.end( ) && entries.pml4.second.accessed >= pml4_threshold ) {
				cache_pml4e.erase( current_physical + 8 * virt_base.pml4_index );
			}

			if ( !entries.pml4.second.value ) {
				cache_pml4e.clear( );
				continue;
			}

			// pdpte
			auto is_pdpte_cached = cache_pdpte.find( ( entries.pml4.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pdpt_index );
			if ( is_pdpte_cached != cache_pdpte.end( ) ) {
				entries.pdpt.second = is_pdpte_cached->second;
				entries.pdpt.second.accessed++;
			} else {
				entries.pdpt.first = reinterpret_cast< ::pdpte* >( ( entries.pml4.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pdpt_index );
				entries.pdpt.second = this->read_kernel_physical<::pdpte>( reinterpret_cast< std::uintptr_t >( entries.pdpt.first ) );

				if ( entries.pdpt.second.value ) {
					cache_pdpte[ ( entries.pml4.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pdpt_index ] = entries.pdpt.second;
					entries.pdpt.second.accessed = 1;
				}
			}

			// Check if the cached entry is not valid and clear it
			if ( is_pdpte_cached != cache_pdpte.end( ) && !entries.pdpt.second.value ) {
				cache_pdpte.erase( ( entries.pml4.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pdpt_index );
			}

			// Check if the access count exceeds the threshold and clear it
			if ( is_pdpte_cached != cache_pdpte.end( ) && entries.pdpt.second.accessed >= pdpte_threshold ) {
				cache_pdpte.erase( ( entries.pml4.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pdpt_index );
			}

			if ( !entries.pdpt.second.value ) {
				cache_pdpte.clear( );
				continue;
			}

			// pd
			auto is_pd_cached = cache_pde.find( ( entries.pdpt.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pd_index );
			if ( is_pd_cached != cache_pde.end( ) ) {
				entries.pd.second = is_pd_cached->second;
				entries.pd.second.accessed++;
			} else {
				entries.pd.first = reinterpret_cast< ::pde* >( ( entries.pdpt.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pd_index );
				entries.pd.second = this->read_kernel_physical<::pde>( reinterpret_cast< std::uintptr_t >( entries.pd.first ) );

				if ( entries.pd.second.value ) {
					cache_pde[ ( entries.pdpt.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pd_index ] = entries.pd.second;
					entries.pd.second.accessed = 1;
				}
			}

			// Check if the cached entry is not valid and clear it
			if ( is_pd_cached != cache_pde.end( ) && !entries.pd.second.value ) {
				cache_pde.erase( ( entries.pdpt.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pd_index );
			}

			// Check if the access count exceeds the threshold and clear it
			if ( is_pd_cached != cache_pde.end( ) && entries.pd.second.accessed >= pd_threshold ) {
				cache_pde.erase( ( entries.pdpt.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pd_index );
			}

			if ( !entries.pd.second.value ) {
				cache_pde.clear( );
				continue;
			}

			// pte
			auto is_pte_cached = cache_pte.find( ( entries.pd.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pt_index );
			if ( is_pte_cached != cache_pte.end( ) ) {
				entries.pt.second = is_pte_cached->second;
				entries.pt.second.accessed++;
			} else {
				entries.pt.first = reinterpret_cast< ::pte* >( ( entries.pd.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pt_index );
				entries.pt.second = this->read_kernel_physical<::pte>( reinterpret_cast< std::uintptr_t >( entries.pt.first ) );

				if ( entries.pt.second.value ) {
					cache_pte[ ( entries.pd.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pt_index ] = entries.pt.second;
					entries.pt.second.accessed = 1;
				}
			}

			// Check if the cached entry is not valid and clear it
			if ( is_pte_cached != cache_pte.end( ) && !entries.pt.second.value ) {
				cache_pte.erase( ( entries.pd.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pt_index );
			}

			// Check if the access count exceeds the threshold and clear it
			if ( is_pte_cached != cache_pte.end( ) && entries.pt.second.accessed >= pte_hreshold ) {
				cache_pte.erase( ( entries.pd.second.pfn << 12 ) + sizeof( std::uintptr_t ) * virt_base.pt_index );
			}

			if ( !entries.pt.second.value ) {
				cache_pte.clear( );
				continue;
			}

			pt_entries entries { };
			auto physical_base = translate_linear(
				entries,
				image_base,
				current_physical
			);
			if ( !physical_base )
				continue;

			//printf("[%i] [physical_base: 0x%llx] [current_physical: 0x%llx]\n", x, physical_base, current_physical );

			//char buffer[ sizeof( IMAGE_DOS_HEADER ) ];
			std::vector<char> buffer(sizeof(IMAGE_DOS_HEADER));
			if ( !read_physical_km( physical_base,buffer.data( ),buffer.size( ) ) )
				continue;

			auto header = reinterpret_cast< IMAGE_DOS_HEADER* >( buffer.data( ) );
			if ( header->e_magic != IMAGE_DOS_SIGNATURE )
				continue;

			return current_physical;
		}
	}

	return 0;
}

auto uefi_interface::c_interface::get_process_pid(
	const std::wstring& proc_name ) -> const std::uint32_t {
	PROCESSENTRY32 proc_info;
	proc_info.dwSize = sizeof( proc_info );

	HANDLE proc_snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS,NULL );
	if ( proc_snapshot == INVALID_HANDLE_VALUE ) {
		return 0;
	}

	Process32First( proc_snapshot,&proc_info );
	if ( !wcscmp( proc_info.szExeFile,proc_name.c_str( ) ) ) {
		CloseHandle( proc_snapshot );
		return proc_info.th32ProcessID;
	}

	while ( Process32Next( proc_snapshot,&proc_info ) ) {
		if ( !wcscmp( proc_info.szExeFile,proc_name.c_str( ) ) ) {
			CloseHandle( proc_snapshot );
			return proc_info.th32ProcessID;
		}
	}

	CloseHandle( proc_snapshot );
	return 0;
}

auto uefi_interface::c_interface::get_build_number(
) -> const std::uint16_t {
	// read peb
	const auto peb = __readgsqword( 0x60 );

	// read peb->OSBuildNumber
	return *reinterpret_cast< std::uint16_t* >( peb + 0x120 );

}

#endif // ! INTERFACE_CPP