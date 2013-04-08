#include "stdafx.h"
#include "GLFunctions.h"

void glColor3(const IEMColoured& c)
{
	glColor4ub(GetRValue(c.getColor()), GetGValue(c.getColor()), GetBValue(c.getColor()), 255);
}

void glColor4(const IEMColouredAndTransparent& ct)
{
	glColor4ub(GetRValue(ct.getColor()), GetGValue(ct.getColor()), GetBValue(ct.getColor()), ct.getAlpha());
}
