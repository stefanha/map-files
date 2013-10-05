#include "map.h"


////////////////////////////////////////////////////////////////////
// Property member functions
////////////////////////////////////////////////////////////////////

void Property::WriteProperty ( ofstream &ofsFile_ ) const
{
/*	Property:
	x char		Property name (zero terminated)
	x char		Property value (zero terminated) */

	ofsFile_ << GetName ( ) << ( char )0x00;
	ofsFile_ << GetValue ( ) << ( char )0x00;

	if ( !IsLast ( ) )
	{
		GetNext ( )->WriteProperty ( ofsFile_ );
	}
}


Property::~Property ( )
{
	if ( m_pacName != NULL )
	{
		delete [] m_pacName;
		m_pacName = NULL;
	}

	if ( m_pacValue != NULL )
	{
		delete [] m_pacValue;
		m_pacValue = NULL;
	}

	if ( m_pNext != NULL )
	{
		delete m_pNext;	// recursively delete whole list
		m_pNext = NULL;
	}
}


Property::Property ( )
{
	m_pacName = NULL;
	m_pacValue = NULL;
	m_pNext = NULL;
}


bool Property::IsLast ( ) const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}


void Property::SetNext ( Property *pProperty_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pProperty_;

		return;
	}

	//
	// Insert the given list
	//
	Property *pProperty = pProperty_;

	while ( !pProperty->IsLast ( ) )
	{
		pProperty = pProperty->GetNext ( );
	}

	pProperty->SetNext ( m_pNext );

	m_pNext = pProperty_;
}


void Property::SetName ( const char *pacName_ )
{
	if ( m_pacName != NULL )
	{
		delete [] m_pacName;
		m_pacName = NULL;
	}

	m_pacName = new char [ strlen ( pacName_ ) + 1 ];

	strcpy ( m_pacName, pacName_ );
}


void Property::SetValue ( const char *pacValue_ )
{
	if ( m_pacValue != NULL )
	{
		delete [] m_pacValue;
		m_pacValue = NULL;
	}

	m_pacValue = new char [ strlen ( pacValue_ ) + 1 ];

	strcpy ( m_pacValue, pacValue_ );
}