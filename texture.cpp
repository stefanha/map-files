#include "map.h"
#include "WAD3.h"


////////////////////////////////////////////////////////////////////
// Texture member functions
////////////////////////////////////////////////////////////////////

Texture* Texture::GetTexture ( char *pacTexture_, LPVOID lpView_, DWORD dwFileSize_, Texture::eGT &rResult_ )
{
	rResult_ = eGT::GT_ERROR;

	//
	// Check if texture already exists
	//
	if ( stricmp ( name, pacTexture_ ) == 0 )
	{
		rResult_ = eGT::GT_FOUND;

		return this;
	}

	if ( !IsLast ( ) )
	{
		return m_pNext->GetTexture ( pacTexture_, lpView_, dwFileSize_, rResult_ );
	}

	//
	// Load texture information
	//
	Texture			*pTexture		= new Texture;
	LPWAD3_HEADER	lpHeader		= NULL;
	LPWAD3_LUMP		lpLump			= NULL;
	LPWAD3_MIP		lpMip			= NULL;

	DWORD			dwNumLumps		= 0;
	DWORD			dwTableOffset	= 0;
	DWORD			dwFilePos		= 0;
	DWORD			dwPaletteOffset	= 0;
	WORD			wPaletteSize	= 0;
	DWORD			dwWidth			= 0;
	DWORD			dwHeight		= 0;

	// Make sure it's at least big enough to manipulate the header
	if (dwFileSize_ < sizeof(WAD3_HEADER))
	{		
		CorruptWAD3( "WAD3 file is malformed.", lpView_);

		delete pTexture;

		return NULL;
	}

	lpHeader = (LPWAD3_HEADER)lpView_;

	if (lpHeader->identification != WAD3_ID)
	{
		CorruptWAD3( "Invalid WAD3 header id.", lpView_);

		delete pTexture;

		return NULL;
	}

	dwNumLumps = lpHeader->numlumps;
	dwTableOffset = lpHeader->infotableofs;

	// Make sure our table is really there
	if ( ((dwNumLumps * sizeof(WAD3_LUMP)) + dwTableOffset) > dwFileSize_)
	{
		CorruptWAD3( "WAD3 file is malformed.", lpView_);

		delete pTexture;

		return NULL;
	}

	// Point at the first table entry
	lpLump = (LPWAD3_LUMP)((LPBYTE)lpView_ + dwTableOffset);

	bool	bFound = false;
	DWORD	j = 0;

	while ( ( !bFound ) && ( j < dwNumLumps ) )
	{		
		if ( lpLump->type == WAD3_TYPE_MIP)
		{
			if ( stricmp ( lpLump->name, pacTexture_ ) == 0 )
			{
				// Find out where the MIP actually is
				dwFilePos = lpLump->filepos;
				
				// Make sure it's in bounds
				if ( dwFilePos >= dwFileSize_ )
				{
					CorruptWAD3( "Invalid lump entry; filepos is malformed.", lpView_ );

					delete pTexture;

					return NULL;
				}

				// Point at the mip
				lpMip = ( LPWAD3_MIP )( ( LPBYTE )lpView_ + dwFilePos );

				strcpy ( pTexture->name, pacTexture_ );

				pTexture->m_iWidth	= lpMip->width;
				pTexture->m_iHeight	= lpMip->height;
				bFound				= true;
			}
		}

		j++;
		lpLump++;
	}

	if ( !bFound )
	{
		delete pTexture;

		return NULL;
	}

	m_pNext = pTexture;

	rResult_ = eGT::GT_LOADED;

	return pTexture;
}


Texture::Texture ( )
{
	memset ( name, 0, MAX_TEXTURE_LENGTH + 1 );

	m_pNext		= NULL;
	m_iHeight	= 0;
	m_iWidth	= 0;
}


Texture::~Texture ( )
{
	if ( !IsLast ( ) )
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


void Texture::SetNext ( Texture *pTexture_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pTexture_;

		return;
	}

	//
	// Insert the given list
	//
	if ( pTexture_ != NULL )
	{
		Texture *pTexture = pTexture_;

		while ( !pTexture->IsLast ( ) )
		{
			pTexture = pTexture->GetNext ( );
		}

		pTexture->SetNext ( m_pNext );
	}

	m_pNext = pTexture_;
}


bool Texture::IsLast ( ) const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}