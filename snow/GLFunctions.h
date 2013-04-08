#pragma once

class IEMColoured
{
public:
	static LPCTSTR tag;
	virtual void setColor(COLORREF color) =0;
	virtual const COLORREF& getColor() const =0;
};

class IEMColouredAndTransparent: public IEMColoured
{
public:
	static LPCTSTR tag;
	virtual void setAlpha(BYTE alpha) =0;
	virtual const BYTE& getAlpha() const =0;
};

template <class BaseClass, COLORREF defaultColor=RGB(0,0,0) >
class EMColoured : public BaseClass
{
public:
	EMColoured()
		:theColor(defaultColor)
	{
	}
	virtual void setColor(COLORREF color) {theColor=color;}
	virtual const COLORREF& getColor() const { return theColor;}
protected:	
	COLORREF theColor;
};

template <class BaseClass, COLORREF defaultColor=RGB(0,0,0), BYTE defaultAlpha=0>
class EMColouredAndTransparent : public EMColoured<BaseClass, defaultColor>
{
public:
	EMColouredAndTransparent()
		:theAlpha(defaultAlpha)
	{
	}
	virtual void setAlpha(BYTE alpha) {theAlpha=alpha;}
	virtual const BYTE& getAlpha() const { return theAlpha;}
protected:	
	BYTE theAlpha;
};


void glColor3(const IEMColoured& c);
void glColor4(const IEMColouredAndTransparent& ct);



