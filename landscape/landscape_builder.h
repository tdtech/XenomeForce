#pragma once

#include "..\layer\render_layer.h"
#include "landscape.h"

class CLandscapeBuilder
{
public:
	CLandscapeBuilder(int width, int height);
	~CLandscapeBuilder();

	/* Restrictions of this method:
	   1. Tile surface must be created with RENDER_FLAG_SYSTEMMEM
	   2. Tile must fit within the landscape */
	// TODO: remove restriction #2
	int addTile(SURFACE_REF tile, int posX, int posY);
	// add more methods...

	ILandscape* buildLandscape();

private:
	int         mWidth;
	int         mHeight;

	SURFACE_REF mLandscapeSurface;
};
