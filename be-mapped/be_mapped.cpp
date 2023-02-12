#include <Windows.h>
#include "dependencies/lazy_importer/lazy_importer.hpp"

void be_mapped( )
{
	constexpr char hey[ ] = { 'h', 'e', 'y', '\0' };
	LI_FN( MessageBoxA )( nullptr, hey, hey, MB_OK );
}