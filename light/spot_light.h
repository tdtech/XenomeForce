#pragma once

#include "..\content\texture_content.h"
#include "..\layer\render_layer.h"
#include "light_manager.h"

#define DEFAULT_SPOT_LIGHT_BRIGHTNESS 200

class CSpotLight: public ILight
{
public:
	CSpotLight(float radius);
	virtual ~CSpotLight();

	inline void setPosition(CVector2D& pos) { mPosition = pos; }
	inline CVector2D& getPosition() { return mPosition; }
	inline void setEnabled(bool enabled) { mEnabled = enabled; }
	virtual void setColor(int brightness, int red, int green, int blue); // [0..255]

	void animate();
	void draw();

protected:
	virtual void doAnimate();
	virtual void doDraw();

protected:
	TSpriteDef       mSpriteDef;

private:
	CVector2D        mPosition;
	float            mRadius;
	bool             mEnabled;

	CTextureContent* mContent;
};