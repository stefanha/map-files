////////////////////////////////////////////////////////////////////
// Filename:	csg.cpp
//
// Author:		Stefan Hajnoczi
//
// Date:		26 May 2001
//
// Description:	Program to parse WorldCraft 3.3 .MAP files and
//				convert them into .CMF files to be used by various
//				BSP tree, PVS, and lighting programs.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

using namespace std;


////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "map.h"


////////////////////////////////////////////////////////////////////
// Function:	WriteCMF
// Description:	Function that saves a .CMF file from a
//				given entity tree.
////////////////////////////////////////////////////////////////////
void WriteCMF ( Entity *pEntity_, Texture *pTextures_, char *pacFilename_ )
{
/*
Header:
	3 char		CMF ID ("CMF")
	1 char		Version byte
	1 uint		Number of WAD files
	1 uint		Number of entities
	1 uint		Number of textures
	x char		WAD filenames (zero terminated)
	x char		Texture names (zero terminated)
	x Entity	Entities
*/

	ofstream	ofsFile;

	ofsFile.open ( pacFilename_, ios::out | ios::binary );

	//
	// Write header ID and version
	//
	ofsFile << "CMF" << ( char )2;

	//
	// Find "wad" property
	//
	Property	*pProperty	= pEntity_->GetProperties ( );
	bool		bFound		= false;

	while ( ( pProperty != NULL ) && ( !bFound ) )
	{
		if ( strcmp ( "wad", pProperty->GetName ( ) ) == 0 )
		{
			bFound = true;
		}
		else
		{
			pProperty = pProperty->GetNext ( );
		}
	}

	if ( !bFound )
	{
		cout << "Unable to find WAD files used in map!" << endl;

		return;
	}

	//
	// Count and write the number of WAD files
	//
	char			acBuffer[ MAX_TOKEN_LENGTH + 1 ];
	const char		*pWAD		= pProperty->GetValue ( );
	unsigned int	uiWADFiles	= 0;
	int				iToken		= 0;

	for ( int i = 0; i < strlen ( pWAD ) + 1; i++ )
	{
		if ( ( pWAD[ i ] == ';' ) || ( pWAD[ i ] == 0x00 ) )
		{
			uiWADFiles++;
		}
	}

	ofsFile.write ( ( char * )&uiWADFiles, sizeof ( uiWADFiles ) );

	//
	// Count and write the number of entities
	//
	Entity			*pEntity	= pEntity_;
	unsigned int	uiEntities	= 0;

	while ( pEntity != NULL )
	{
		uiEntities++;

		pEntity = pEntity->GetNext ( );
	}

	ofsFile.write ( ( char * )&uiEntities, sizeof ( uiEntities ) );

	//
	// Count and write the number of textures
	//
	Texture			*pTexture	= pTextures_;
	unsigned int	uiTextures	= 0;

	while ( pTexture != NULL )
	{
		uiTextures++;

		pTexture = pTexture->GetNext ( );
	}

	ofsFile.write ( ( char * )&uiTextures, sizeof ( uiTextures ) );

	//
	// Write the WAD filenames
	//
	memset ( acBuffer, 0, MAX_TOKEN_LENGTH + 1 );

	for ( i = 0; i < strlen ( pWAD ) + 1; i++ )
	{
		if ( ( pWAD[ i ] == ';' ) || ( pWAD[ i ] == 0x00 ) )
		{
			ofsFile << acBuffer << ( char )0x00;

			iToken = 0;
			memset ( acBuffer, 0, MAX_TOKEN_LENGTH + 1 );
		}
		else
		{
			acBuffer[ iToken ] = pWAD[ i ];
			iToken++;
		}
	}

	//
	// Write the texture names
	//
	pTexture = pTextures_;

	for ( i = 0; i < uiTextures; i++ )
	{
		ofsFile << pTexture->name << ( char )0x00;

		pTexture = pTexture->GetNext ( );
	}

	//
	// Write the entities
	//
	pEntity_->WriteEntity ( ofsFile );

	//
	// Clean up and return
	//
	ofsFile.close ( );
}


////////////////////////////////////////////////////////////////////
// Function:	main
// Description:	Program to load, parse, CSG union, and save
//				WorldCraft 3.3 .MAP files as .CMF files.
////////////////////////////////////////////////////////////////////
void main ( int nArgs_, char *pcArgs[ ] )
{
	//
	// If in debug mode, turn on memory leak checking
	//
#ifdef _DEBUG
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag

	flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit

	_CrtSetDbgFlag(flag); // Set flag to the new value
#endif


	//
	// Make sure parameters are valid
	//
	if ( nArgs_ != 3 )
	{
		cout << "csg in out" << endl;
		cout << "in\t- MAP file" << endl;
		cout << "out\t- CMF file" << endl;

		return;
	}


	//
	// Parse .MAP file
	//
	Entity	*pEntities = NULL;
	Texture *pTextures = NULL;
	MAPFile	map;

	cout << "Parsing " << pcArgs[ 1 ] << "..." << endl;
	if ( !map.Load ( pcArgs[ 1 ], &pEntities, &pTextures ) )
	{
		cout << "Error parsing " << pcArgs[ 1 ] << "!" << endl;

		return;
	}


	//
	// Save .CMF file
	//
	WriteCMF ( pEntities, pTextures, pcArgs[ 2 ] );


#ifdef _DEBUG
	//
	// Debug dump
	//
	ofstream	ofs ( "Debug.txt" );
	Entity		*pEntity = pEntities;

	while ( pEntity != NULL )
	{
		ofs << "{" << endl;

		Property	*pProperty = pEntity->GetProperties ( );

		while ( pProperty != NULL )
		{
			ofs << pProperty->GetName ( ) << " = " << pProperty->GetValue ( ) << endl;

			pProperty = pProperty->GetNext ( );
		}

		Poly		*pPoly = pEntity->GetPolys ( );

		while ( pPoly != NULL )
		{
			ofs << endl;
			ofs << "tID = " << pPoly->TextureID << endl;
			ofs << "plane = " << pPoly->plane.n.x << "x + " << pPoly->plane.n.y << "y + " << pPoly->plane.n.z << "z + " << pPoly->plane.d << " = 0" << endl;

			for ( int i = 0; i < pPoly->GetNumberOfVertices ( ); i++ )
			{
				ofs << pPoly->verts[ i ].p.x << ", " << pPoly->verts[ i ].p.y << ", " << pPoly->verts[ i ].p.z << " [" << pPoly->verts[ i ].tex[ 0 ] << ", " << pPoly->verts[ i ].tex[ 1 ] << "]" << endl;
			}

			pPoly = pPoly->GetNext ( );
		}

		ofs << "}" << endl;

		pEntity = pEntity->GetNext ( );
	}
#endif


	//
	// Clean up and quit
	//
	delete pEntities;
	delete pTextures;

	return;
}
