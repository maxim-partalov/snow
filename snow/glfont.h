// Simbeor Electromagnetic Simulation Environment
//
// Copyright (c) Simberian Inc. 2006-2010. All rights reserved.
// This work contains trade secret and proprietary information which is 
// the property of Simberian Inc or its licensors and is subject to license terms.
//
// Revision history:
//
//////////////////////////////////////////////////////////////////////////////////////

#pragma once

class CGLFont
{
public:
	CGLFont(TCHAR cShift, GLsizei iCount, int iType=-1);
	~CGLFont();
	//выдел€ет слоты дл€ глифов и заполн€ет их пустыми глифами, 
	//нужно освобождать через вызов reset
	//даже если ни разу не вызывалс€ update
	void init();
	
	//апдейтит источник накоплени€ глифов
	//если источник накоплени€ не изменилс€, то вообще ничего не происходит
	//если источник накоплени€ изменилс€ то удал€ютс€ ресурсы системы захваченные 
	//старыми глифами и прописываетс€ новый источник накоплени€ глифов
	bool update(HDC hdc, const LOGFONT& logfont); 
	
	//освобождает ресурсы системы захваченные в процессе накоплени€ глифов
	void reset();
	
	//выводит текст и накапливает глифы соотвествующие источнику накоплени€
	void print(LPCTSTR psz);
	void setParPer(int iPar, int iPer) {m_iPar=iPar; m_iPer=iPer;}
	void setHorizontalShift(int iHorizontalShift) {m_iHorizontalShift=iHorizontalShift;}
	void setVerticalShift(int iVerticalShift) {m_iVerticalShift=iVerticalShift;}
	
	void setBitmapHeightShift(short sHeightShift){m_sHeightShift=sHeightShift;}
	void setOutlineHeightShift(float fHeightShift){m_fHeightShift=fHeightShift;}

	short getBitmapHeightShift(){return m_sHeightShift;}
	float getOutlineHeightShift(){return m_fHeightShift;}
	
	void getBitmapDimensions(LPCTSTR psz, short& sLength, short& sHeight);
	void getOutlineDimensions(LPCTSTR psz, float& fLength, float& fHeight);
private:
	int m_iType;
	GLuint list(TCHAR c);
	GLuint	m_uiFontBase;
	LOGFONT	m_logfont;
	TCHAR m_cShift;
	GLsizei m_iCount;
	HDC m_hdc;
	GLYPHMETRICSFLOAT* m_pGMF;
	GLYPHMETRICS* m_pGM;
	int m_iPar; //0==left, 1==center, 2==right
	int m_iPer; //0==bottom, 1==middle, 2==top
	int m_iHorizontalShift;
	int m_iVerticalShift;
	MAT2 m_mat2;
	short m_sHeightShift;
	float m_fHeightShift;
	HFONT m_font;
};

