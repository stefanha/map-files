//////////////////////////////////////////////////////////////////////////////////////////
//	Wally dudes:  ty@wwa.com, neal_white_iii@hotmail.com
//
//	01/08/2001
//
//	Restrictions:  None... do whatever you want with this!
//
//	This is a basic WAD3 reader.  It converts all of the images in a WAD to PCX files.
//	Assumes the current working directory for output, but the function supports any old
//	directory.  I didn't want to bother with checking for the silly backslashes and such,
//	so that's your fun part :)
//
//	This code is completely unsupported, and was written purely for other people to
//	learn the WAD3 structure.  That said, it *does* work and is a useful tool all by
//	itself.  Of course the really cool stuff we do inside Wally, but that wouldn't be
//	so easy to duplicate here.  I will answer any reasonable questions about the 
//	format, or even anything else in this code, but I make no warranties about using
//	this as the base for another program.  IE, test the crap out of what you're 
//	coding and really make sure it's working the way it should be.  
//
//	I put this together in a short amount of time, so there may be bugs or such.  
//	It's also not very optimized, but it runs fast enough to not bother me too 
//	much :)  I am doing a *lot* of bounds checking, so make sure you keep that stuff 
//	in there.  It is always better to be safe than sorry, and you never know when 
//	something might be corrupt.
//
//	Since this code only reads data and doesn't actually write a WAD file (that's 
//	for you to play with <g>) it doesn't bother looking at some important items.  
//	Specifically, the WORD padding at the very end of a mip's data block.  This WORD 
//	padding isn't used for anything, but it HAS to be there.  Check out the pseudocode 
//	structure in the header file for more details there.  If you come up with a WAD3
//	creation utility, just make sure Wally and WorldCraft can load your WAD, and you'll
//	be good to go.
//
//	- Ty
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "WAD3.h"
#include "stdio.h"

void MapFile( LPCTSTR szFileName, LPVOID *pView, LPDWORD pdwFileSize )
{
	HANDLE hFile = NULL;
	HANDLE hFileMapping = NULL;
	LPVOID lpView = NULL;
	DWORD dwFileSize = 0;
	DWORD dwError = 0;

	hFile = CreateFile( 
		szFileName,					// Name of file
		GENERIC_READ,				// Desired access
		FILE_SHARE_READ,			// Share mode
		NULL,						// Security attributes
		OPEN_EXISTING,				// Creation disposition
		FILE_FLAG_SEQUENTIAL_SCAN,	// Attributes and flags
		NULL);						// Template file	

	if (hFile == INVALID_HANDLE_VALUE)
	{
		// CreateFile() does not return NULL on error
		throw CWADException();
	}

	// Store this away for now...
	dwFileSize = GetFileSize( hFile, NULL);

	if (dwFileSize == -1)
	{
		dwError = ::GetLastError();
		CloseHandle( hFile);
		throw CWADException( dwError);
	}
	
	hFileMapping = CreateFileMapping( 
		hFile,						// Handle to file
		NULL,						// Security attributes
		PAGE_READONLY,				// Protection
		0,							// Max size high
		0,							// Max size low
		NULL);						// Name of mapping object	
	
	if (hFileMapping == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFile);
		throw CWADException();
	}

	// We don't need this anymore
	CloseHandle( hFile);

	// Map to the entire file
	lpView = MapViewOfFile(
		hFileMapping,				// Handle to the mapping
		FILE_MAP_READ,				// Desired access
		0,							// Offset high
		0,							// Offset low
		0);							// Number of bytes

	if (lpView == NULL)
	{
		dwError = ::GetLastError();
		CloseHandle( hFileMapping);
		throw CWADException( dwError);
	}

	// We don't need this anymore
	CloseHandle( hFileMapping);

	if (pView)
	{
		*pView = lpView;
	}
	
	if (pdwFileSize)
	{
		*pdwFileSize = dwFileSize;
	}
}

int CorruptWAD3( LPCTSTR szErrorMessage, LPVOID lpView)
{
	UnmapViewOfFile( lpView);
	throw CWADException( szErrorMessage);

	// We never actually get here, but oh well
	return 1;
}