#pragma once

#include <windows.h>

#include "..\common\xmath.h"

#define MAX_ADAPTER_DESCRIPTION_LENGTH 512
#define MAX_DEVICE_NAME_LENGTH 32

#define MAX_SURFACES 128
#define MAX_TEXTURES 128

#define MAX_VERTEX_COUNT 64

// Flags
#define RENDER_FLAG_NONE         0x0000
#define RENDER_FLAG_SYSTEMMEM    0x0001
#define RENDER_FLAG_VIDEOMEM     0x0002
#define RENDER_FLAG_MANAGEDMEM   0x0004
#define RENDER_FLAG_RENDERTARGET 0x0008

typedef unsigned int SURFACE_REF;
typedef unsigned int TEXTURE_REF;

// Possible display formats
enum TFormat
{
	 EFormatR3G3B2 = 0
	,EFormatX1R5G5B5
	,EFormatA1R5G5B5
	,EFormatA4R4G4B4
	,EFormatR5G6B5
	,EFormatX8R8G8B8
	,EFormatA8R8G8B8
	// add more formats...
	,EFormatCount // This item always should be the last
};

// Possible rasterization types
enum TRasterizationType
{
	 ERasterizationTypeSoftware
	,ERasterizationTypeHardware
};

// Display mode structure
struct TDisplayMode
{
	unsigned int  mWidth;
	unsigned int  mHeight;
	unsigned int  mRefreshRate;
	TFormat       mFormat;
};

// Adapter info structure
struct TAdapterInfo
{
	char          mDescription[MAX_ADAPTER_DESCRIPTION_LENGTH];
	char          mDeviceName[MAX_DEVICE_NAME_LENGTH];
	TDisplayMode* mDisplayMode;
	unsigned int  mDisplayModeCount;
};

// 2D Vertex + Texture coordinates (U, V)
struct TVertex
{
	float mX;
	float mY;
	float mU;
	float mV;
};

// Sprite definition
struct TSpriteDef
{
	CVector2D     mLeftTop;
	float         mWidth;
	float         mHeight;
	float         mAngle;
	unsigned long mDiffuse;
	TRect         mTextureRect;

	// TODO: try to find common drawing algorythm instead of dividing sprites into two types.
	bool          mTexelToPixel; // set to true if you want to use 1:1 texel-to-pixel convertion
};

// Device listener
class IRenderDeviceListener
{
public:
	virtual ~IRenderDeviceListener() {}

	virtual void onRenderDeviceLost() = 0;
	virtual void onRenderDeviceReset() = 0;
};

// Render layer interface
class IRenderLayer
{
public:

// Adapter related methods
	virtual unsigned int getAdapterCount() = 0;
	virtual int getAdapterInfo(unsigned int adapter, TAdapterInfo& adapterInfo) = 0;
	// TODO: do we really need this method?
	virtual int getAdapterDisplayMode(unsigned int adapter, TDisplayMode& displayMode) = 0;

// Device related methods
	virtual int validateDeviceCapabilities(unsigned int adapter, TRasterizationType rasterization) = 0;
	virtual int initDevice(unsigned int adapter, TRasterizationType rasterization, const TDisplayMode& displayMode) = 0;
	virtual int getDeviceDisplayMode(TDisplayMode& displayMode) = 0;
	virtual int getMaxTextureSize(unsigned long& maxWidth, unsigned long& maxHeight) = 0;
	virtual int beginScene() = 0; // begin pushing data to device
	virtual int endScene() = 0; // submit pushed data to device, must be called as far ahead of calling present as possible
	virtual int presentScene() = 0; // present pushed data to monitor
	virtual int setRenderTarget(SURFACE_REF surface) = 0;
	virtual int setRenderTarget(SURFACE_REF surface, TRect& clipBound) = 0;
	virtual int setDefaultRenderTarget() = 0;
	virtual int addDeviceListener(IRenderDeviceListener* listener) = 0; // returns unique id
	virtual int removeDeviceListener(int id) = 0;

// Color related methods
	virtual unsigned long ARGB(int a, int r, int g, int b) = 0;

// Text related methods
	virtual int setFont(int width, int height, const char* faceName) = 0;
	virtual int beginText() = 0;
	virtual int endText() = 0;
	virtual int outText(const char* text, TRect& destRect, unsigned long color) = 0;

// Surface related methods
	virtual int createSurface(unsigned int width, unsigned int height, unsigned long flags, SURFACE_REF& surface) = 0;
	virtual int createSurfaceFromFile(char* fileName, unsigned int width, unsigned int height, TRect& srcRect, unsigned long flags, SURFACE_REF& surface) = 0;
	virtual int incSurfaceRef(SURFACE_REF surface) = 0;
	virtual int fillSurface(SURFACE_REF surface, unsigned long color) = 0;
	virtual int drawSurface(SURFACE_REF surface) = 0;
	virtual int drawSurface(SURFACE_REF surface, long posX, long posY, TRect& srcRect) = 0;
	virtual int drawSurface(SURFACE_REF surface, long posX, long posY) = 0;
	virtual int drawSurface(SURFACE_REF surface, TRect& destRect, TRect& srcRect) = 0;
	virtual int drawSurface(SURFACE_REF surface, TRect& srcRect) = 0;
	virtual int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface) = 0;
	virtual int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY, TRect& srcRect) = 0;
	virtual int drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY) = 0;
	virtual int releaseSurface(SURFACE_REF surface) = 0;

// Texture related methods
	virtual int createTexture(unsigned int width, unsigned int height, unsigned long flags, TEXTURE_REF& texture) = 0;
	virtual int createTextureFromFile(char* fileName, unsigned long colorKey, unsigned long flags, unsigned int& width, unsigned int& height, TEXTURE_REF& texture) = 0;
	virtual int incTextureRef(TEXTURE_REF texture) = 0;
	virtual int beginSprite() = 0;
	virtual int endSprite() = 0;
	virtual int getSpriteScale(CVector2D& scale) = 0;
	virtual int setSpriteScale(CVector2D& scale) = 0; // scaling also applies to position
	virtual int drawSprite(TEXTURE_REF texture, const TSpriteDef& spriteDef) = 0;
	virtual int drawTexture(TEXTURE_REF srcTexture, TEXTURE_REF destTexture) = 0;
	virtual int getSurfaceFromTexture(TEXTURE_REF texture, SURFACE_REF& surface) = 0; // IMPORTANT: obtained surface MUST be released before releasing texture itself
	virtual int releaseTexture(TEXTURE_REF texture) = 0;

// Primitives (polygons and etc.)
	virtual int beginPrimitive() = 0;
	virtual int endPrimitive() = 0;
	virtual int drawPolygon(TEXTURE_REF texture, TVertex* vertices, unsigned int vertexCount) = 0;

	virtual int release() = 0;
// add more methods...

protected:
	virtual ~IRenderLayer() {}
};

// ==========================================================

// Methods to obtain render layers
IRenderLayer* obtainDirectXRenderLayer(HWND hwnd);