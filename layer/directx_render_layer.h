#pragma once

#include <d3dx9.h>

#include "..\common\collection.h"
#include "render_layer.h"

#define MAX_HELPER_RECTS 2

struct TSurfaceHolder
{
	LPDIRECT3DSURFACE9 mD3DSurface;
	ULONG              mInitialRefCount;
};

struct TTextureHolder
{
	LPDIRECT3DTEXTURE9 mD3DTexture;
	float              mInvWidth;
	float              mInvHeight;
};

class CDirectXRenderLayer: public IRenderLayer
{
public:
	static CDirectXRenderLayer* obtainInstance(HWND hwnd);

// Adapter related methods
	unsigned int getAdapterCount();
	int getAdapterInfo(unsigned int adapter, TAdapterInfo& adapterInfo);
	int getAdapterDisplayMode(unsigned int adapter, TDisplayMode& displayMode);

// Device related methods
	int validateDeviceCapabilities(unsigned int adapter, TRasterizationType rasterization);
	int initDevice(unsigned int adapter, TRasterizationType rasterization, const TDisplayMode& displayMode);
	int getDeviceDisplayMode(TDisplayMode& displayMode);
	int getMaxTextureSize(unsigned long& maxWidth, unsigned long& maxHeight);
	int beginScene();
	int endScene();
	int presentScene();
	int setRenderTarget(SURFACE_REF surface);
	int setRenderTarget(SURFACE_REF surface, TRect& clipBound);
	int setDefaultRenderTarget();
	int addDeviceListener(IRenderDeviceListener* listener);
	int removeDeviceListener(int id);

// Color related methods
	unsigned long ARGB(int a, int r, int g, int b);

// Text related methods
	int setFont(int width, int height, const char* faceName);
	int beginText();
	int endText();
	int outText(const char* text, TRect& destRect, unsigned long color);

// Surface related methods
	int createSurface(unsigned int width, unsigned int height, unsigned long flags, SURFACE_REF& surface);
	int createSurfaceFromFile(char* fileName, unsigned int width, unsigned int height, TRect& srcRect, unsigned long flags, SURFACE_REF& surface);
	int incSurfaceRef(SURFACE_REF surface);
	int fillSurface(SURFACE_REF surface, unsigned long color);
	int drawSurface(SURFACE_REF surface);
	int drawSurface(SURFACE_REF surface, long posX, long posY, TRect& srcRect);
	int drawSurface(SURFACE_REF surface, long posX, long posY);
	int drawSurface(SURFACE_REF surface, TRect& destRect, TRect& srcRect);
	int drawSurface(SURFACE_REF surface, TRect& srcRect);
	int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface);
	int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY, TRect& srcRect);
	int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY);
	int releaseSurface(SURFACE_REF surface);

// Texture related methods
	int createTexture(unsigned int width, unsigned int height, unsigned long flags, TEXTURE_REF& texture);
	int createTextureFromFile(char* fileName, unsigned long colorKey, unsigned long flags, unsigned int& width, unsigned int& height, TEXTURE_REF& texture);
	int incTextureRef(TEXTURE_REF texture);
	int beginSprite();
	int endSprite();
	int getSpriteScale(CVector2D& scale);
	int setSpriteScale(CVector2D& scale);
	int drawSprite(TEXTURE_REF texture, const TSpriteDef& spriteDef);
	int drawTexture(TEXTURE_REF srcTexture, TEXTURE_REF destTexture);
	int getSurfaceFromTexture(TEXTURE_REF texture, SURFACE_REF& surface);
	int releaseTexture(TEXTURE_REF texture);

// Primitives (polygons and etc.)
	int beginPrimitive();
	int endPrimitive();
	int drawPolygon(TEXTURE_REF texture, TVertex* vertices, unsigned int vertexCount);

	int release();
// add more methods...

private:
	CDirectXRenderLayer(HWND hwnd);
	~CDirectXRenderLayer();

	struct TVertexData
	{
		float mX;
		float mY;
		float mZ;
		float mRhw;
		DWORD mDiffuse;
		float mU;
		float mV;
	};

	void releaseResources();
	D3DFORMAT convertTFormatToD3DFormat(TFormat aFormat);
	D3DPOOL convertRenderFlagToD3DPool(unsigned long aFlags);
	unsigned long convertRenderFlagToD3DUsage(unsigned long aFlags);
	D3DDEVTYPE convertRasterizationTypeToDevType(TRasterizationType rasterization);
	int findFreeSurfaceRef(SURFACE_REF& ref);
	int findFreeTextureRef(TEXTURE_REF& ref);

	int validateDeviceStatus();
	int resetDevice();

	void notifyOnDeviceLost();
	void notifyOnDeviceReset();

private:
	LPDIRECT3D9                        mD3DInterface;
	LPDIRECT3DDEVICE9                  mD3DDevice;
	LPD3DXSPRITE                       mD3DFontSprite;
	LPD3DXFONT                         mD3DFont;

	LPDIRECT3DSURFACE9                 mDefaultBackBuffer;
	LPDIRECT3DSURFACE9                 mBackBuffer;

	TSurfaceHolder                     mSurface[MAX_SURFACES];
	TTextureHolder                     mTexture[MAX_TEXTURES];

	LPDIRECT3DVERTEXBUFFER9            mVertexBuffer;
	LPDIRECT3DINDEXBUFFER9             mIndexBuffer;

	D3DPRESENT_PARAMETERS              mPresentParams;
	D3DFORMAT                          mCurrentFormat;

	HWND                               mHwnd;

	D3DCAPS9                           mDeviceCaps;
	TDisplayMode                       mDeviceDisplayMode;

	DWORD                              mMaxPrimitiveCount;

	// Sprite batch
	CVector2D                          mSpriteScale;
	TVertexData                        mQuad[4];
	TEXTURE_REF                        mSpriteTextureRef;
	int                                mPrimitiveCount;

	// Device listeners
	CCollection<IRenderDeviceListener> mDeviceListeners;

	// Helpers
	RECT                               mRect[MAX_HELPER_RECTS];
	POINT                              mPoint;
	TVertexData*                       mVertexData;
	D3DSURFACE_DESC                    mSurfaceDescriptor;
	D3DVIEWPORT9                       mD3DViewPort;
	bool                               mIsDeviceLostHandled;

	static CDirectXRenderLayer*        mInstance;
	static int                         mRefCount;
};