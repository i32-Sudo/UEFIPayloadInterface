#include <iostream>
#include "communication/edk.hpp"
#define LocalPlayer  0x23e7ff8
#define ENTITYLSIT  0x1f14e08

int main()
{
	m_vm.get()->attach_vm((L"r5apex.exe"));

	while (true)
	{
		std::cout << m_vm.get()->m_image_base << "\n";
		for (int i = 0; i <= 255; i++)
		{
			uint64_t localplayerPTR = m_vm.get()->read<std::uintptr_t>(m_vm.get()->m_image_base + 0x22b1528);//OFFSET_LOCAL_ENT
			std::cout << "LocalPlayer 0x" << std::hex << (uint64_t)localplayerPTR << "\n";
		}
	}
	


	
}