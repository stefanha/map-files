#include "map.h"


////////////////////////////////////////////////////////////////////
// Poly member functions
////////////////////////////////////////////////////////////////////

void Poly::WritePoly ( ofstream &ofsFile_ ) const
{
/*
Polygon:
	1 uint		Texture ID
	1 Plane		Polygon plane
	1 uint		Number of vertices
	x Vertex	Vertices
*/

	ofsFile_.write ( ( char * )&TextureID, sizeof ( unsigned int ) );
	ofsFile_.write ( ( char * )&plane.n.x, sizeof ( double ) );
	ofsFile_.write ( ( char * )&plane.n.y, sizeof ( double ) );
	ofsFile_.write ( ( char * )&plane.n.z, sizeof ( double ) );
	ofsFile_.write ( ( char * )&plane.d, sizeof ( double ) );

	unsigned int ui = ( unsigned int )GetNumberOfVertices ( );

	ofsFile_.write ( ( char * )&ui, sizeof ( ui ) );

	for ( int i = 0; i < GetNumberOfVertices ( ); i++ )
	{
		ofsFile_.write ( ( char * )&verts[ i ].p.x, sizeof ( double ) );
		ofsFile_.write ( ( char * )&verts[ i ].p.y, sizeof ( double ) );
		ofsFile_.write ( ( char * )&verts[ i ].p.z, sizeof ( double ) );
		ofsFile_.write ( ( char * )&verts[ i ].tex[ 0 ], sizeof ( double ) );
		ofsFile_.write ( ( char * )&verts[ i ].tex[ 1 ], sizeof ( double ) );
	}

	if ( !IsLast ( ) )
	{
		GetNext ( )->WritePoly ( ofsFile_ );
	}
}


const bool Poly::operator == ( const Poly &arg_ ) const
{
	if ( m_iNumberOfVertices == arg_.m_iNumberOfVertices )
	{
		if ( plane.d == arg_.plane.d )
		{
			if ( plane.n == arg_.plane.n )
			{
				for ( int i = 0; i < GetNumberOfVertices ( ); i++ )
				{
					if ( verts[ i ].p == arg_.verts[ i ].p )
					{
						if ( verts[ i ].tex[ 0 ] != arg_.verts[ i ].tex[ 0 ] )
						{
							return false;
						}

						if ( verts[ i ].tex[ 1 ] != arg_.verts[ i ].tex[ 1 ] )
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}

				if ( TextureID == arg_.TextureID )
				{
					return true;
				}
			}
		}
	}

	return false;
}


Poly *Poly::ClipToList ( Poly *pPoly_, bool bClipOnPlane_ )
{
	switch ( ClassifyPoly ( pPoly_ ) )
	{
	case eCP::FRONT:
		{
			return pPoly_->CopyPoly ( );
		} break;

	case eCP::BACK:
		{
			if ( IsLast ( ) )
			{
				return NULL;
			}

			return m_pNext->ClipToList ( pPoly_, bClipOnPlane_ );
		} break;

	case eCP::ONPLANE:
		{
			double	Angle = plane.n.Dot ( pPoly_->plane.n ) - 1;

			if ( ( Angle < epsilon ) && ( Angle > -epsilon ) )
			{
				if ( !bClipOnPlane_ )
				{
					return pPoly_->CopyPoly ( );
				}
			}

			if ( IsLast ( ) )
			{
				return NULL;
			}

			return m_pNext->ClipToList ( pPoly_, bClipOnPlane_ );
		} break;

	case eCP::SPLIT:
		{
			Poly *pFront	= NULL;
			Poly *pBack		= NULL;

			SplitPoly ( pPoly_, &pFront, &pBack );

			if ( IsLast ( ) )
			{
				delete pBack;

				return pFront;
			}

			Poly *pBackFrags = m_pNext->ClipToList ( pBack, bClipOnPlane_ );

			if ( pBackFrags == NULL )
			{
				delete pBack;

				return pFront;
			}

			if ( *pBackFrags == *pBack )
			{
				delete pFront;
				delete pBack;
				delete pBackFrags;

				return pPoly_->CopyPoly ( );
			}

			delete pBack;

			pFront->AddPoly ( pBackFrags );

			return pFront;
		} break;
	}

	return NULL;
}


Poly *Poly::CopyPoly ( ) const
{
	Poly *pPoly = new Poly;

	pPoly->TextureID = TextureID;

	pPoly->m_iNumberOfVertices = m_iNumberOfVertices;
	pPoly->plane = plane;

	pPoly->verts = new Vertex[ m_iNumberOfVertices ];
	memcpy ( pPoly->verts, verts, sizeof ( Vertex ) * m_iNumberOfVertices );

	return pPoly;
}


Poly *Poly::CopyList ( ) const
{
	Poly *pPoly = new Poly;

	pPoly->TextureID = TextureID;

	pPoly->m_iNumberOfVertices = m_iNumberOfVertices;
	pPoly->plane = plane;

	pPoly->verts = new Vertex[ m_iNumberOfVertices ];
	memcpy ( pPoly->verts, verts, sizeof ( Vertex ) * m_iNumberOfVertices );

	if ( !IsLast ( ) )
	{
		pPoly->AddPoly ( m_pNext->CopyList ( ) );
	}

	return pPoly;
}


Poly::eCP Poly::ClassifyPoly ( Poly *pPoly_ )
{
	bool	bFront = false, bBack = false;
	double	dist;

	for ( int i = 0; i < ( int )pPoly_->GetNumberOfVertices ( ); i++ )
	{
		dist = plane.n.Dot ( pPoly_->verts[ i ].p ) + plane.d;

		if ( dist > 0.001 )
		{
			if ( bBack )
			{
				return eCP::SPLIT;
			}

			bFront = true;
		}
		else if ( dist < -0.001 )
		{
			if ( bFront )
			{
				return eCP::SPLIT;
			}

			bBack = true;
		}
	}

	if ( bFront )
	{
		return eCP::FRONT;
	}
	else if ( bBack )
	{
		return eCP::BACK;
	}

	return eCP::ONPLANE;
}


void Poly::SplitPoly ( Poly *pPoly_, Poly **ppFront_, Poly **ppBack_ )
{
	Plane::eCP	*pCP = new Plane::eCP[ pPoly_->GetNumberOfVertices ( ) ];

	//
	// Classify all points
	//
	for ( int i = 0; i < pPoly_->GetNumberOfVertices ( ); i++ )
	{
		pCP[ i ] = plane.ClassifyPoint ( pPoly_->verts[ i ].p );
	}

	//
	// Build fragments
	//
	Poly		*pFront = new Poly;
	Poly		*pBack	= new Poly;

	pFront->TextureID	= pPoly_->TextureID;
	pBack->TextureID	= pPoly_->TextureID;
	pFront->plane		= pPoly_->plane;
	pBack->plane		= pPoly_->plane;

	for ( i = 0; i < pPoly_->GetNumberOfVertices ( ); i++ )
	{
		//
		// Add point to appropriate list
		//
		switch ( pCP[ i ] )
		{
		case Plane::eCP::FRONT:
			{
				pFront->AddVertex ( pPoly_->verts[ i ] );
			} break;

		case Plane::eCP::BACK:
			{
				pBack->AddVertex ( pPoly_->verts[ i ] );
			} break;

		case Plane::eCP::ONPLANE:
			{
				pFront->AddVertex ( pPoly_->verts[ i ] );
				pBack->AddVertex ( pPoly_->verts[ i ] );
			} break;
		}

		//
		// Check if edges should be split
		//
		int		iNext	= i + 1;
		bool	bIgnore	= false;

		if ( i == ( pPoly_->GetNumberOfVertices ( ) - 1 ) )
		{
			iNext = 0;
		}

		if ( ( pCP[ i ] == Plane::eCP::ONPLANE ) && ( pCP[ iNext ] != Plane::eCP::ONPLANE ) )
		{
			bIgnore = true;
		}
		else if ( ( pCP[ iNext ] == Plane::eCP::ONPLANE ) && ( pCP[ i ] != Plane::eCP::ONPLANE ) )
		{
			bIgnore = true;
		}

		if ( ( !bIgnore ) && ( pCP[ i ] != pCP[ iNext ] ) )
		{
			Vertex	v;	// New vertex created by splitting
			double	p;	// Percentage between the two points

			plane.GetIntersection ( pPoly_->verts[ i ].p, pPoly_->verts[ iNext ].p, v.p, p );

			v.tex[ 0 ] = pPoly_->verts[ iNext ].tex[ 0 ] - pPoly_->verts[ i ].tex[ 0 ];
			v.tex[ 1 ] = pPoly_->verts[ iNext ].tex[ 1 ] - pPoly_->verts[ i ].tex[ 1 ];

			v.tex[ 0 ] = pPoly_->verts[ i ].tex[ 0 ] + ( p * v.tex[ 0 ] );
			v.tex[ 1 ] = pPoly_->verts[ i ].tex[ 1 ] + ( p * v.tex[ 1 ] );

			pFront->AddVertex ( v );
			pBack->AddVertex ( v );
		}
	}

	delete [] pCP;

	pFront->CalculatePlane ( );
	pBack->CalculatePlane ( );

	*ppFront_ = pFront;
	*ppBack_ = pBack;
}


void Poly::CalculateTextureCoordinates ( int texWidth, int texHeight, Plane texAxis[ 2 ], double texScale[ 2 ] )
{
	//
	// Calculate texture coordinates
	//
	for ( int i = 0; i < GetNumberOfVertices ( ); i++ )
	{
		double U, V;
		
		U = texAxis[ 0 ].n.Dot ( verts[ i ].p );
		U = U / ( ( double )texWidth ) / texScale[ 0 ];
		U = U + ( texAxis[ 0 ].d / ( double )texWidth );

		V = texAxis[ 1 ].n.Dot ( verts[ i ].p );
		V = V / ( ( double )texHeight ) / texScale[ 1 ];
		V = V + ( texAxis[ 1 ].d / ( double )texHeight );

		verts[ i ].tex[ 0 ] = U;
		verts[ i ].tex[ 1 ] = V;
	}

	//
	// Check which axis should be normalized
	//
	bool	bDoU = true;
	bool	bDoV = true;

	for ( i = 0; i < GetNumberOfVertices ( ); i++ )
	{
		if ( ( verts[ i ].tex[ 0 ] < 1 ) && ( verts[ i ].tex[ 0 ] > -1 ) )
		{
			bDoU = false;
		}

		if ( ( verts[ i ].tex[ 1 ] < 1 ) && ( verts[ i ].tex[ 1 ] > -1 ) )
		{
			bDoV = false;
		}
	}

	//
	// Calculate coordinate nearest to 0
	//
	if ( bDoU || bDoV )
	{
		double	NearestU = 0;
		double	U = verts[ 0 ].tex[ 0 ];

		double	NearestV = 0;
		double	V = verts[ 0 ].tex[ 1 ];

		if ( bDoU )
		{
			if ( U > 1 )
			{
				NearestU = floor ( U );
			}
			else
			{
				NearestU = ceil ( U );
			}
		}

		if ( bDoV )
		{
			if ( V > 1 )
			{
				NearestV = floor ( V );
			}
			else
			{
				NearestV = ceil ( V );
			}
		}

		for ( i = 0; i < GetNumberOfVertices ( ); i++ )
		{
			if ( bDoU )
			{
				U = verts[ i ].tex[ 0 ];

				if ( fabs ( U ) < fabs ( NearestU ) )
				{
					if ( U > 1 )
					{
						NearestU = floor ( U );
					}
					else
					{
						NearestU = ceil ( U );
					}
				}
			}

			if ( bDoV )
			{
				V = verts[ i ].tex[ 1 ];

				if ( fabs ( V ) < fabs ( NearestV ) )
				{
					if ( V > 1 )
					{
						NearestV = floor ( V );
					}
					else
					{
						NearestV = ceil ( V );
					}
				}
			}
		}

		//
		// Normalize texture coordinates
		//
		for ( i = 0; i < GetNumberOfVertices ( ); i++ )
		{
			verts[ i ].tex[ 0 ] = verts[ i ].tex[ 0 ] - NearestU;
			verts[ i ].tex[ 1 ] = verts[ i ].tex[ 1 ] - NearestV;
		}
	}
}


void Poly::SortVerticesCW ( )
{
	//
	// Calculate center of polygon
	//
	Vector3	center;

	for ( int i = 0; i < GetNumberOfVertices ( ); i++ )
	{
		center = center + verts[ i ].p;
	}

	center = center / GetNumberOfVertices ( );

	//
	// Sort vertices
	//
	for ( i = 0; i < GetNumberOfVertices ( ) - 2; i++ )
	{
		Vector3	a;
		Plane	p;
		double	SmallestAngle	= -1;
		int		Smallest		= -1;

		a = verts[ i ].p - center;
		a.Normalize ( );

		p.PointsToPlane ( verts[ i ].p, center, center + plane.n );

		for ( int j = i + 1; j < GetNumberOfVertices ( ); j++ )
		{
			if ( p.ClassifyPoint ( verts[ j ].p ) != Plane::eCP::BACK )
			{
				Vector3	b;
				double	Angle;
				
				b = verts[ j ].p - center;
				b.Normalize ( );

				Angle = a.Dot ( b );

				if ( Angle > SmallestAngle )
				{
					SmallestAngle	= Angle;
					Smallest		= j;
				}
			}
		}

		if ( Smallest == -1 )
		{
			cout << "Error: Degenerate polygon!" << endl;

			abort ( );
		}

		Vertex	t			= verts[ Smallest ];
		verts[ Smallest ]	= verts[ i + 1 ];
		verts[ i + 1 ]		= t;
	}

	//
	// Check if vertex order needs to be reversed for back-facing polygon
	//
	Plane	oldPlane = plane;

	CalculatePlane ( );

	if ( plane.n.Dot ( oldPlane.n ) < 0 )
	{
		int j = GetNumberOfVertices ( );

		for ( int i = 0; i < j / 2; i++ )
		{
			Vertex v			= verts[ i ];
			verts[ i ]			= verts[ j - i - 1 ];
			verts[ j - i - 1 ]	= v;
		}
	}
}


bool Poly::CalculatePlane ( )
{
    Vector3	centerOfMass;
    double	magnitude;
    int     i, j;

    if ( GetNumberOfVertices ( ) < 3 )
	{
		cout << "Polygon has less than 3 vertices!" << endl;

		return false;
	}

    plane.n.x		= 0.0f;
    plane.n.y		= 0.0f;
    plane.n.z		= 0.0f;
    centerOfMass.x	= 0.0f; 
    centerOfMass.y	= 0.0f; 
    centerOfMass.z	= 0.0f;

    for ( i = 0; i < GetNumberOfVertices ( ); i++ )
    {
        j = i + 1;

        if ( j >= GetNumberOfVertices ( ) )
		{
			j = 0;
		}

        plane.n.x += ( verts[ i ].p.y - verts[ j ].p.y ) * ( verts[ i ].p.z + verts[ j ].p.z );
        plane.n.y += ( verts[ i ].p.z - verts[ j ].p.z ) * ( verts[ i ].p.x + verts[ j ].p.x );
        plane.n.z += ( verts[ i ].p.x - verts[ j ].p.x ) * ( verts[ i ].p.y + verts[ j ].p.y );

        centerOfMass.x += verts[ i ].p.x;
        centerOfMass.y += verts[ i ].p.y;
        centerOfMass.z += verts[ i ].p.z;
    }

    if ( ( fabs ( plane.n.x ) < epsilon ) && ( fabs ( plane.n.y ) < epsilon ) &&
		 ( fabs ( plane.n.z ) < epsilon ) )
    {
        return false;
    }

    magnitude = sqrt ( plane.n.x * plane.n.x + plane.n.y * plane.n.y + plane.n.z * plane.n.z );

    if ( magnitude < epsilon )
	{
		return false;
	}

    plane.n.x /= magnitude;
    plane.n.y /= magnitude;
    plane.n.z /= magnitude;

    centerOfMass.x /= ( double )GetNumberOfVertices ( );
    centerOfMass.y /= ( double )GetNumberOfVertices ( );
    centerOfMass.z /= ( double )GetNumberOfVertices ( );

    plane.d = -( centerOfMass.Dot ( plane.n ) );

    return true;
}


void Poly::AddPoly ( Poly *pPoly_ )
{
	if ( pPoly_ != NULL )
	{
		if ( IsLast ( ) )
		{
			m_pNext = pPoly_;

			return;
		}

		Poly *pPoly = m_pNext;

		while ( !pPoly->IsLast ( ) )
		{
			pPoly = pPoly->GetNext ( );
		}

		pPoly->m_pNext = pPoly_;
	}
}


void Poly::SetNext ( Poly *pPoly_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pPoly_;

		return;
	}

	//
	// Insert the given list
	//
	Poly *pPoly = pPoly_;

	while ( !pPoly->IsLast ( ) )
	{
		pPoly = pPoly->GetNext ( );
	}

	pPoly->SetNext ( m_pNext );

	m_pNext = pPoly_;
}


void Poly::AddVertex ( Vertex &Vertex_ )
{
	Vertex *pVertices = new Vertex[ m_iNumberOfVertices + 1 ];

	memcpy ( pVertices, verts, sizeof ( Vertex ) * m_iNumberOfVertices );

	delete [] verts;

	verts = pVertices;

	verts[ m_iNumberOfVertices ] = Vertex_;

	m_iNumberOfVertices++;
}


bool Poly::IsLast () const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}


Poly::Poly ( )
{
	m_pNext				= NULL;
	verts				= NULL;
	m_iNumberOfVertices	= 0;
	TextureID			= 0;
}


Poly::~Poly ( )
{
	if ( !IsLast ( ) )
	{
		delete m_pNext;
		m_pNext = NULL;
	}

	if ( verts != NULL )
	{
		delete [] verts;
		verts = NULL;
		m_iNumberOfVertices = 0;
	}
}