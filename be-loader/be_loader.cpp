#include "dependencies/util/util.h"
#include "dll.hpp"

void* loader_base;

void( *load_image_original )( const UNICODE_STRING&, HANDLE, const IMAGE_INFO& );
void( *create_thread_original )( HANDLE, HANDLE, BOOLEAN );

uintptr_t load_image, create_thread;

void create_thread_hook( const HANDLE process_id, const HANDLE thread_id, const BOOLEAN create )
{
	if ( const auto process_name = reinterpret_cast< const char* >( reinterpret_cast< uintptr_t >( PsGetCurrentProcess( ) ) + 0x5A8 ); !strcmp( process_name, "Unturned.exe" ) )
	{
		create_thread_original( process_id, thread_id, create );

		UnTrampolineHook( create_thread, create_thread_original );

		DbgPrint( "[be-loader] process: %s (0x%X)", process_name, process_id );

		void* dll_base = nullptr;
		auto dll_size = sizeof( dll );

		ZwAllocateVirtualMemory( reinterpret_cast< HANDLE >( -1 ), &dll_base, 0, &dll_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

		DbgPrint( "[be-loader] dll_base: 0x%p", dll_base );

		memcpy( dll_base, dll, dll_size );

		ExFreePool( loader_base );

		return;
	}

	create_thread_original( process_id, thread_id, create );
}

void load_image_hook( const UNICODE_STRING& image_name, const HANDLE process_id, const IMAGE_INFO& image_info )
{
	if ( wcsstr( image_name.Buffer, L"BEDaisy.sys" ) )
	{
		const auto image_base = reinterpret_cast< uintptr_t >( image_info.ImageBase );

		DbgPrint( "[be-loader] bedaisy: 0x%p", image_base );

		load_image_original( image_name, process_id, image_info );

		UnTrampolineHook( load_image, load_image_original );

		create_thread = image_base + 0x323C9C;
		DbgPrint( "[be-loader] create_thread: 0x%p", create_thread );

		TrampolineHook( &create_thread_hook, create_thread, reinterpret_cast< void** >( &create_thread_original ) );

		return;
	}

	load_image_original( image_name, process_id, image_info );
}

NTSTATUS be_loader( void* loader_base )
{
	load_image = FindPatternImage( GetBaseAddress( "ahcache.sys", nullptr ), "\x48\x85\xC9\x0F\x84\x72\x01\x00\x00\x4C\x8B\xDC\x55\x41\x56\x41\x57\x48\x83\xEC\x60\x45\x33\xFF", "xxxxxxxxxxxxxxxxxxxxxxxx" );
	DbgPrint( "[be-loader] load_image: 0x%p", load_image );

	TrampolineHook( &load_image_hook, load_image, reinterpret_cast< void** >( &load_image_original ) );

	::loader_base = loader_base;
	DbgPrint( "[be-loader] loader_base: 0x%p", loader_base );
	
	return STATUS_SUCCESS;
}