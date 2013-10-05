#pragma once


////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////

class Vertex
{
public:
	Vector3	p;
	double	tex[ 2 ];
};


class Texture
{
private:
	Texture*		m_pNext;
	int				m_iWidth;
	int				m_iHeight;

public:
	enum eGT { GT_FOUND = 0, GT_LOADED, GT_ERROR };

	unsigned int	uiID;	
	char			name[ MAX_TEXTURE_LENGTH + 1 ];

	Texture ( );
	~Texture ( );

	Texture* GetTexture ( char *pacTexture_, LPVOID lpView_, DWORD dwFileSize_, eGT &rResult_ );
	Texture* GetNext ( ) const { return m_pNext; }

	bool IsLast ( ) const;
	int GetHeight ( ) const { return m_iHeight; }
	int GetWidth ( ) const { return m_iWidth; }

	void SetNext ( Texture* pTexture_ );
};


class Poly
{
public:
	enum eCP
	{
		FRONT = 0, SPLIT, BACK, ONPLANE
	};

	Vertex			*verts;
	Plane			plane;
	unsigned int	TextureID;

	Poly *GetNext ( ) const { return m_pNext; }
	Poly *CopyList ( ) const;
	Poly *CopyPoly ( ) const;
	Poly *ClipToList ( Poly *pPoly_, bool bClipOnPlane_ );

	int GetNumberOfVertices ( ) const { return m_iNumberOfVertices; }

	void AddVertex ( Vertex &Vertex_ );
	void AddPoly ( Poly *pPoly_ );
	void SetNext ( Poly *pPoly_ );
	void WritePoly ( ofstream &ofsFile_ ) const;

	bool CalculatePlane ( );
	void SortVerticesCW ( );
	void ToLeftHanded ( );
	void CalculateTextureCoordinates ( int texWidth, int texHeight, Plane texAxis[ 2 ], double texScale[ 2 ] );
	void Poly::SplitPoly ( Poly *pPoly_, Poly **ppFront_, Poly **ppBack_ );
	eCP ClassifyPoly ( Poly *pPoly_ );

	bool IsLast ( ) const;

	const bool operator == ( const Poly &arg_ ) const;

	Poly ( );
	~Poly ( );

private:
	Poly	*m_pNext;
	int		m_iNumberOfVertices;
};


class Property
{
private:
	char		*m_pacName;		// Property's name (zero terminated string)
	char		*m_pacValue;	// Property's value (zero terminated string)
	Property	*m_pNext;		// Next property in linked list

public:
	const char *GetName ( ) const { return m_pacName; }
	const char *GetValue ( ) const { return m_pacValue; }
	Property *GetNext ( ) const { return m_pNext; }

	void SetName ( const char *pacName_ );
	void SetValue ( const char *pacValue_ );
	void SetNext ( Property *pProperty_ );
	void WriteProperty ( ofstream &ofsFile_ ) const;

	bool IsLast ( ) const;

	Property ( );
	~Property ( );
};


class Entity
{
private:
	Entity		*m_pNext;
	Property	*m_pProperties;
	Poly		*m_pPolys;

public:
	Entity *GetNext ( ) const { return m_pNext; }
	Property *GetProperties ( ) { return m_pProperties; }
	Poly *GetPolys ( ) const { return m_pPolys; }

	unsigned int GetNumberOfProperties ( ) const;
	unsigned int GetNumberOfPolys ( ) const;

	void AddEntity ( Entity *pEntity_ );
	void AddProperty ( Property *pProperty_ );
	void AddPoly ( Poly *pPoly_ );
	void WriteEntity ( ofstream& ofsFile_ ) const;

	bool IsLast ( ) const;

	Entity ( );
	~Entity ( );
};
