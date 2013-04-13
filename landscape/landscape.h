#pragma once

#include "..\common\xmath.h"

class ILandscape
{
public:
	virtual ~ILandscape() {}

	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual void draw(TRect& rect) = 0;

	virtual void beginUpdate() = 0;
	virtual void endUpdate() = 0;
};