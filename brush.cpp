#include "map.h"


////////////////////////////////////////////////////////////////////
// Brush member functions
////////////////////////////////////////////////////////////////////

void Brush::ClipToBrush ( Brush *pBrush_, bool bClipOnPlane_ )
{
	Poly *pPolyList = NULL;
	Poly *pPoly		= m_pPolys;

	for ( int i = 0; i < GetNumberOfPolys ( ); i++ )
	{
		Poly *pClippedPoly = pBrush_->GetPolys ( )->ClipToList ( pPoly, bClipOnPlane_ );

		if ( pPolyList == NULL )
		{
			pPolyList = pClippedPoly;
		}
		else
		{
			pPolyList->AddPoly ( pClippedPoly );
		}

		pPoly = pPoly->GetNext ( );
	}

	delete m_pPolys;
	m_pPolys = pPolyList;
}


Poly *Brush::MergeList ( )
{
	Brush			*pClippedList	= CopyList ( );
	Brush			*pClip			= pClippedList;
	Brush			*pBrush			= NULL;
	Poly			*pPolyList		= NULL;

	bool			bClipOnPlane	= false;
	unsigned int	uiBrushes		= GetNumberOfBrushes ( );

	for ( int i = 0; i < uiBrushes; i++ )
	{
		pBrush			= this;
		bClipOnPlane	= false;

		for ( int j = 0; j < uiBrushes; j++ )
		{
			if ( i == j )
			{
				bClipOnPlane = true;
			}
			else
			{
				if ( pClip->AABBIntersect ( pBrush ) )
				{
					pClip->ClipToBrush ( pBrush, bClipOnPlane );
				}
			}

			pBrush = pBrush->GetNext ( );
		}

		pClip = pClip->GetNext ( );
	}

	pClip = pClippedList;

	while ( pClip != NULL )
	{
		if ( pClip->GetNumberOfPolys ( ) != 0 )
		{
			//
			// Extract brushes left over polygons and add them to the list
			//
			Poly *pPoly = pClip->GetPolys ( )->CopyList ( );

			if ( pPolyList == NULL )
			{
				pPolyList = pPoly;
			}
			else
			{
				pPolyList->AddPoly ( pPoly );
			}

			pClip = pClip->GetNext ( );
		}
		else
		{
			//
			// Brush has no polygons and should be deleted
			//
			if ( pClip == pClippedList )
			{
				pClip = pClippedList->GetNext ( );

				pClippedList->SetNext ( NULL );

				delete pClippedList;

				pClippedList = pClip;
			}
			else
			{
				Brush	*pTemp = pClippedList;

				while ( pTemp != NULL )
				{
					if ( pTemp->GetNext ( ) == pClip )
					{
						break;
					}

					pTemp = pTemp->GetNext ( );
				}

				pTemp->m_pNext = pClip->GetNext ( );
				pClip->SetNext ( NULL );

				delete pClip;

				pClip = pTemp->GetNext ( );
			}
		}
	}

	delete pClippedList;

	return pPolyList;
}


Brush *Brush::CopyList ( ) const
{
	Brush *pBrush = new Brush;

	pBrush->max = max;
	pBrush->min = min;

	pBrush->m_pPolys = m_pPolys->CopyList ( );

	if ( !IsLast ( ) )
	{
		pBrush->SetNext ( m_pNext->CopyList ( ) );
	}

	return pBrush;
}


bool Brush::AABBIntersect ( Brush *pBrush_ )
{
	if ( ( min.x > pBrush_->max.x ) || ( pBrush_->min.x > max.x ) )
	{
		return false;
	}

	if ( ( min.y > pBrush_->max.y ) || ( pBrush_->min.y > max.y ) )
	{
		return false;
	}

	if ( ( min.z > pBrush_->max.z ) || ( pBrush_->min.z > max.z ) )
	{
		return false;
	}

	return true;
}


void Brush::CalculateAABB ( )
{
	min = m_pPolys->verts[ 0 ].p;
	max = m_pPolys->verts[ 0 ].p;

	Poly *pPoly = m_pPolys;

	for ( int i = 0; i < GetNumberOfPolys ( ); i++ )
	{
		for ( int j = 0; j < pPoly->GetNumberOfVertices ( ); j++ )
		{
			//
			// Calculate min
			//
			if ( pPoly->verts[ j ].p.x < min.x )
			{
				min.x = pPoly->verts[ j ].p.x;
			}

			if ( pPoly->verts[ j ].p.y < min.y )
			{
				min.y = pPoly->verts[ j ].p.y;
			}

			if ( pPoly->verts[ j ].p.z < min.z )
			{
				min.z = pPoly->verts[ j ].p.z;
			}

			//
			// Calculate max
			//
			if ( pPoly->verts[ j ].p.x > max.x )
			{
				max.x = pPoly->verts[ j ].p.x;
			}

			if ( pPoly->verts[ j ].p.y > max.y )
			{
				max.y = pPoly->verts[ j ].p.y;
			}

			if ( pPoly->verts[ j ].p.z > max.z )
			{
				max.z = pPoly->verts[ j ].p.z;
			}
		}

		pPoly = pPoly->GetNext ( );
	}
}


Brush::Brush ( )
{
	m_pNext = NULL;
	m_pPolys = NULL;
}


Brush::~Brush ( )
{
	if ( m_pPolys != NULL )
	{
		delete m_pPolys;
		m_pPolys = NULL;
	}

	if ( !IsLast ( ) )
	{
		delete m_pNext;
		m_pNext = NULL;
	}
}


void Brush::AddPoly ( Poly *pPoly_ )
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


unsigned int Brush::GetNumberOfBrushes ( ) const
{
	Brush			*pBrush		= m_pNext;
	unsigned int	uiCount		= 1;

	while ( pBrush != NULL )
	{
		pBrush = pBrush->GetNext ( );
		uiCount++;
	}

	return uiCount;
}


unsigned int Brush::GetNumberOfPolys ( ) const
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


void Brush::SetNext ( Brush *pBrush_ )
{
	if ( IsLast ( ) )
	{
		m_pNext = pBrush_;

		return;
	}

	if ( pBrush_ == NULL )
	{
		m_pNext = NULL;
	}
	else
	{
		Brush *pBrush = pBrush_;

		while ( !pBrush->IsLast ( ) )
		{
			pBrush = pBrush->GetNext ( );
		}

		pBrush->SetNext ( m_pNext );

		m_pNext = pBrush_;
	}
}


bool Brush::IsLast ( ) const
{
	if ( m_pNext == NULL )
	{
		return true;
	}

	return false;
}