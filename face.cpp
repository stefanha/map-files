#include "map.h"


////////////////////////////////////////////////////////////////////
// Face member functions
////////////////////////////////////////////////////////////////////

Poly *Face::GetPolys ( )
{
	//
	// Create the polygons from the faces
	//
	unsigned int	uiFaces = 0;
	Face			*pFace	= this;

	while ( pFace != NULL )
	{
		pFace = pFace->GetNext ( );
		uiFaces++;
	}

	Poly			*pPolyList	= NULL;
	Face			*lfi		= NULL;
	Face			*lfj		= NULL;
	Face			*lfk		= NULL;

	//
	// Create polygons
	//
	pFace = this;

	for ( int c = 0; c < uiFaces; c++ )
	{
		if ( pPolyList == NULL )
		{
			pPolyList = new Poly;
		}
		else
		{
			pPolyList->AddPoly ( new Poly );
		}

		if ( c == uiFaces - 3 )
		{
			lfi = pFace->GetNext ( );
		}
		else if ( c == uiFaces - 2 )
		{
			lfj = pFace->GetNext ( );
		}
		else if ( c == uiFaces - 1 )
		{
			lfk = pFace->GetNext ( );
		}

		pFace = pFace->GetNext ( );
	}

	//
	// Loop through faces and create polygons
	//
	Poly	*pi	= pPolyList;

	for ( Face *fi = this; fi != lfi; fi = fi->GetNext ( ) )
	{
		Poly	*pj = pi->GetNext ( );

		for ( Face *fj = fi->GetNext ( ); fj != lfj; fj = fj->GetNext ( ) )
		{
			Poly	*pk = pj->GetNext ( );

			for ( Face *fk = fj->GetNext ( ); fk != lfk; fk = fk->GetNext ( ) )
			{
				Vector3 p;

				if ( fi->plane.GetIntersection ( fj->plane, fk->plane, p ) )
				{
					Face *f = this;

					while ( true )
					{
						if ( f->plane.ClassifyPoint ( p ) == Plane::eCP::FRONT )
						{
							break;
						}

						if ( f->IsLast ( ) )	// The point is not outside the brush
						{
							Vertex v;

							v.p = p;

							pi->AddVertex ( v );
							pj->AddVertex ( v );
							pk->AddVertex ( v );

							break;
						}

						f = f->GetNext ( );
					}
				}

				pk = pk->GetNext ( );
			}

			pj = pj->GetNext ( );
		}

		pi = pi->GetNext ( );
	}

	return pPolyList;
}


Face::Face ( )
{
	m_pNext = NULL;
}


Face::~Face ( )
{
	if ( !IsLast ( ) )
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


bool Face::IsLast ( ) const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}


void Face::SetNext ( Face *pFace_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pFace_;

		return;
	}

	//
	// Insert the given list
	//
	Face *pFace = pFace_;

	while ( !pFace->IsLast ( ) )
	{
		pFace = pFace->GetNext ( );
	}

	pFace->SetNext ( m_pNext );

	m_pNext = pFace_;
}


void Face::AddFace ( Face *pFace_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pFace_;

		return;
	}

	Face *pFace = m_pNext;

	while ( !pFace->IsLast ( ) )
	{
		pFace = pFace->GetNext ( );
	}

	pFace->m_pNext = pFace_;
}