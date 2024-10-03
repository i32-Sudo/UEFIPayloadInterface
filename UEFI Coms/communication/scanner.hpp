#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>

#define in_range(x,a,b)    (x >= a && x <= b) 
#define get_bits( x )    (in_range((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xA) : (in_range(x,'0','9') ? x - '0' : 0))
#define get_byte( x )    (get_bits(x[0]) << 4 | get_bits(x[1]))

namespace scanner
{
	inline uintptr_t pattern_scan( uint32_t start, uint32_t size, const char* pattern ) {
		auto find_pattern = [ = ]( uint32_t start, uint32_t size, const char* signature ) {
			const char* pat = signature;
			uint32_t match = 0;
			uint32_t module_start = start;

			uint32_t module_end = module_start + size;

			for ( uint32_t current = module_start; current < module_end; current++ ) {
				if ( !*pat )
					return match;

				if ( *( uint8_t* ) pat == '\?' || *( uint8_t* ) current == get_byte( pat ) ) {
					if ( !match )
						match = current;

					if ( !pat[2] )
						return match;

					if ( *( uint16_t* ) pat == '\?\?' || *( uint8_t* ) pat != '\?' )
						pat += 3;
					else
						pat += 2;
				}
				else {
					pat = signature;
					match = 0;
				}
			}

			return 0u;
			};

		auto buf = new uint8_t[size];
		int chunk_size = 0x1000;
		for ( int i = 0; i < size; i += chunk_size )
			m_vm.get( )->read_large_array( start + i, buf + i, chunk_size );

		auto result = find_pattern( ( uint32_t ) buf, size, pattern );
		if ( !result ) {
			delete[] buf;
			return 0;
		}
		result -= ( uint32_t ) buf;
		result += start;

		delete[] buf;

		return result;
	}

	inline uintptr_t find_pattern( uintptr_t base, const char* pattern )
	{
		IMAGE_DOS_HEADER DOSHeader = m_vm.get( )->read<IMAGE_DOS_HEADER>( base );
		IMAGE_NT_HEADERS NTHeaders = m_vm.get( )->read<IMAGE_NT_HEADERS>( base + DOSHeader.e_lfanew );

		return pattern_scan(
			reinterpret_cast< uintptr_t >( base ) + NTHeaders.OptionalHeader.BaseOfCode, reinterpret_cast< uintptr_t >( base ) + NTHeaders.OptionalHeader.SizeOfCode, pattern );
	}
}

#endif // include guard