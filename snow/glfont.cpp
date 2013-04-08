// Simbeor Electromagnetic Simulation Environment
//
// Copyright (c) Simberian Inc. 2006-2010. All rights reserved.
// This work contains trade secret and proprietary information which is 
// the property of Simberian Inc or its licensors and is subject to license terms.
//
// Revision history:
//
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "glfont.h"
#include <math.h>

#define LOG_WRITE

FIXED FixedFromDouble(const double& d)
{
	long l;
	l = (long) (d * 65536L);
	return *(FIXED *)&l;
}

CGLFont::CGLFont(TCHAR cShift, GLsizei iCount, int iType)
	:m_iType(iType)
	,m_uiFontBase(0)
	,m_cShift(cShift)
	,m_iCount(iCount)
	,m_hdc(NULL)
	,m_pGMF(NULL)
	,m_pGM(NULL)
	,m_iPar(0)
	,m_iPer(0)
	,m_iHorizontalShift(0)
	,m_iVerticalShift(0)
	,m_sHeightShift(0)
	,m_fHeightShift(0.0f)
	,m_font(NULL)
{
	double dRotationAngel=0.0;//M_PI_2;

	m_mat2.eM11 = FixedFromDouble(cos(dRotationAngel));
	m_mat2.eM12 = FixedFromDouble(sin(dRotationAngel));
	m_mat2.eM21 = FixedFromDouble(-sin(dRotationAngel));
	m_mat2.eM22 = FixedFromDouble(cos(dRotationAngel));

	memset(&m_logfont, 0, sizeof(LOGFONT));

	if(iType<0)
	{
		m_pGM=new  GLYPHMETRICS[iCount];
	}

	if(iType>=0)
	{
		m_pGMF=new GLYPHMETRICSFLOAT[iCount];
	}
}
CGLFont::~CGLFont()
{
	if(m_pGMF)
	{
		delete[] m_pGMF;
	}
	if(m_pGM)
	{
		delete[] m_pGM;
	}
}
void CGLFont::init()
{
	m_uiFontBase = ::glGenLists(m_iCount);
}
bool CGLFont::update(HDC hdc, const LOGFONT& logfont)
{
	if(m_uiFontBase)
	{
		m_hdc=hdc;	
		if(memcmp(&m_logfont, &logfont, sizeof(LOGFONT))!=0)
		{
			::glDeleteLists(m_uiFontBase, m_iCount);
			memcpy(&m_logfont, &logfont, sizeof(LOGFONT));
			
			//---
			if(m_font)
			{
				::DeleteObject(m_font);
				m_font=NULL;
			}
			//---
			
			//---
			m_font = ::CreateFontIndirect(&m_logfont);
			//---

			if(m_pGMF)
			{
				memset(m_pGMF, 0, sizeof(GLYPHMETRICSFLOAT)*m_iCount);
			}
			if(m_pGM)
			{
				memset(m_pGM, 0, sizeof(GLYPHMETRICS)*m_iCount);
			}
			return true;
		}
	}
	return false;
}
void CGLFont::reset()
{
	if(m_uiFontBase)
	{
		//---
		if(m_font)
		{
			::DeleteObject(m_font);
			m_font=NULL;
		}
		//---

		m_hdc=0;	
		::glDeleteLists(m_uiFontBase, m_iCount);
		memset(&m_logfont, 0, sizeof(LOGFONT)); 
		
		if(m_pGMF)
		{
			memset(m_pGMF, 0, sizeof(GLYPHMETRICSFLOAT)*m_iCount);
		}
		if(m_pGM)
		{
			memset(m_pGM, 0, sizeof(GLYPHMETRICS)*m_iCount);
		}
	}
}

void CGLFont::getBitmapDimensions(LPCTSTR psz, short& sLength, short& sHeight)
{
	sLength=0;
	sHeight=0;

	if(psz==NULL)
	{
		return;
	}

	LPCTSTR p=psz;
	while(*p)
	{
		if(!glIsList(list(*p)))
		{
			HFONT font = (HFONT)::SelectObject(m_hdc, m_font);
			if(::wglUseFontBitmaps(m_hdc, *p, 1, list(*p))==FALSE)
			{
				if(::wglUseFontBitmaps(m_hdc, *p, 1, list(*p))==FALSE)
				{
					LOG_WRITE(_T("wglUseFontBitmaps failed for character: %c"), *p);
				}
			}
			if(::GetGlyphOutline(m_hdc, *p, GGO_METRICS, &m_pGM[*p-m_cShift], 0, NULL, &m_mat2)==GDI_ERROR)
			{
				LOG_WRITE(_T("GetGlyphOutline failed for character: %c"), *p);
			}
			::SelectObject(m_hdc, font);
		}
		++p;
	}

	p=psz;
	while(*p)
	{
		sLength+=m_pGM[*p-m_cShift].gmCellIncX;
		sHeight=max((UINT)sHeight, m_pGM[*p-m_cShift].gmBlackBoxY);
		++p;
	}
}
void CGLFont::getOutlineDimensions(LPCTSTR psz, float& fLength, float& fHeight)
{
	fLength=0.0f;
	fHeight=0.0f;

	if(psz==NULL)
	{
		return;
	}

	LPCTSTR p=psz;
	while(*p)
	{
		if(!glIsList(list(*p)))
		{
			HFONT font = (HFONT)::SelectObject(m_hdc, m_font);
			if(::wglUseFontOutlines(m_hdc, *p, 1, list(*p), 0.0f, 0.0f, m_iType, &m_pGMF[*p-m_cShift])==FALSE)
			{
				LOG_WRITE(_T("wglUseFontOutlines failed for character: %c"), *p);
			}
			::SelectObject(m_hdc, font);
		}
		++p;
	}

	
	p=psz;
	while(*p)
	{
		fLength+=m_pGMF[*p-m_cShift].gmfCellIncX;
		fHeight=max(fHeight, m_pGMF[*p-m_cShift].gmfBlackBoxY);
		++p;
	}
}

void CGLFont::print(LPCTSTR psz)
{
	if(m_pGM)
	{
		short sLength;
		short sHeight;
		getBitmapDimensions(psz, sLength, sHeight);

		GLubyte fake[8]={0};
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glBitmap(8, 8, 0.0f, 0.0f, -0.5f*sLength*m_iPar -0.25f*m_sHeightShift*m_iHorizontalShift, -0.5f*m_sHeightShift*m_iPer - 0.25f*m_sHeightShift*m_iVerticalShift, fake);
	}

	if(m_pGMF)
	{
		float fLength;
		float fHeight;
		getOutlineDimensions(psz, fLength, fHeight);
		
		glTranslatef(-0.5f*fLength*m_iPar-0.5f*m_fHeightShift*m_iHorizontalShift, -0.5f*m_fHeightShift*m_iPer-0.25f*m_fHeightShift*m_iVerticalShift, 0.0f);	
	}

	::glPushAttrib(GL_LIST_BIT);
	::glListBase(m_uiFontBase-m_cShift);

#ifdef _UNICODE
	::glCallLists((GLsizei)_tcslen(psz), GL_UNSIGNED_SHORT, psz);
#else
	::glCallLists(_tcslen(psz), GL_UNSIGNED_BYTE, psz);
#endif

	::glPopAttrib();
}
GLuint CGLFont::list(TCHAR c)
{
	return m_uiFontBase+c-m_cShift;
}
