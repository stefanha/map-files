#include "map.h"


////////////////////////////////////////////////////////////////////
// Entity member functions
////////////////////////////////////////////////////////////////////

void Entity::WriteEntity ( ofstream &ofsFile_ ) const
{
/*	Entity:
	x char		Entity class (zero terminated)
	1 uint		Number of properties
	x Property	Entities properties
	1 uint		Number of polygons
	x Polygon	Polygons */

	ofsFile_ << m_pProperties->GetValue ( ) << ( char )0x00;

	unsigned int ui = GetNumberOfProperties ( ) - 1;

	ofsFile_.write ( ( char * )&ui, sizeof ( ui ) );

	if ( !m_pProperties->IsLast ( ) )
	{
		m_pProperties->GetNext ( )->WriteProperty ( ofsFile_ );
	}

	ui = GetNumberOfPolys ( );

	ofsFile_.write ( ( char * )&ui, sizeof ( ui ) );

	if ( GetNumberOfPolys ( ) > 0 )
	{
		m_pPolys->WritePoly ( ofsFile_ );
	}

	if ( !IsLast ( ) )
	{
		GetNext ( )->WriteEntity ( ofsFile_ );
	}
}


Entity::Entity ( )
{
	m_pNext			= NULL;
	m_pProperties	= NULL;
	m_pPolys		= NULL;
}


Entity::~Entity ( )
{
	if ( m_pProperties != NULL )
	{
		delete m_pProperties;
		m_pProperties = NULL;
	}

	if ( m_pPolys != NULL )
	{
		delete m_pPolys;
		m_pPolys = NULL;
	}

	if ( m_pNext != NULL )
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


bool Entity::IsLast ( ) const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}


void Entity::AddEntity ( Entity *pEntity_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pEntity_;

		return;
	}

	Entity *pEntity = m_pNext;

	while ( !pEntity->IsLast ( ) )
	{
		pEntity = pEntity->GetNext ( );
	}

	pEntity->m_pNext = pEntity_;
}


void Entity::AddProperty ( Property *pProperty_ )
{
	if ( m_pProperties == NULL )
	{
		m_pProperties = pProperty_;

		return;
	}

	Property *pProperty = m_pProperties;

	while ( !pProperty->IsLast () )
	{
		pProperty = pProperty->GetNext ( );
	}

	pProperty->SetNext ( pProperty_ );
}


void Entity::AddPoly ( Poly *pPoly_ )
{
	if ( m_pPolys == NULL )
	{
		m_pPolys = pPoly_;

		return;
	}

	Poly *pPoly = m_pPolys;

	while ( !pPoly->IsLast ( ) )
	{
		pPoly = pPoly->GetNext ( );
	}

	pPoly->SetNext ( pPoly_ );
}


unsigned int Entity::GetNumberOfProperties ( ) const
{
	Property		*pProperty	= m_pProperties;
	unsigned int	uiCount		= 0;

	while ( pProperty != NULL )
	{
		pProperty = pProperty->GetNext ( );
		uiCount++;
	}

	return uiCount;
}


unsigned int Entity::GetNumberOfPolys ( ) const
{
	Poly			*pPoly		= m_pPolys;
	unsigned int	uiCount		= 0;

	while ( pPoly != NULL )
	{
		pPoly = pPoly->GetNext ( );
		uiCount++;
	}

	return uiCount;
}