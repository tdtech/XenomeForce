#include "..\common\common.h"
#include "..\log\logger.h"
#include "directx_render_layer.h"

#define LISTENERS_INITIAL_CAPACITY 16

#define MIN_TEXTURE_SIZE 512
#define MAX_SPRITES_PER_TEXTURE (MAX_VERTEX_COUNT * 4)

#define VERTEX_BUFFER_LENGTH (MAX_SPRITES_PER_TEXTURE * 4) // vertex count
#define VERTEX_BUFFER_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#if ((VERTEX_BUFFER_LENGTH - 1) > 0x0000FFFF)
#define USE_32BIT_INDEX_BUFFER
#endif

CDirectXRenderLayer* CDirectXRenderLayer::mInstance = NULL;
int CDirectXRenderLayer::mRefCount = 0;

CDirectXRenderLayer* CDirectXRenderLayer::obtainInstance(HWND hwnd)
{
	if (mInstance == NULL) {
		mInstance = new CDirectXRenderLayer(hwnd);
	}

	mRefCount++;
	return mInstance;
}

CDirectXRenderLayer::CDirectXRenderLayer(HWND hwnd):
mD3DInterface(NULL),
mD3DDevice(NULL),
mD3DFontSprite(NULL),
mD3DFont(NULL),
mDefaultBackBuffer(NULL),
mBackBuffer(NULL),
mVertexBuffer(NULL),
mIndexBuffer(NULL),
mCurrentFormat(D3DFMT_X8R8G8B8),
mHwnd(hwnd),
mMaxPrimitiveCount(0),
mSpriteScale(1.0f, 1.0f), // No sprite scale initially
mSpriteTextureRef(MAX_TEXTURES),
mPrimitiveCount(0),
mDeviceListeners(LISTENERS_INITIAL_CAPACITY),
mVertexData(NULL),
mIsDeviceLostHandled(false)
{
	int i;
	for (i = 0; i < MAX_SURFACES; i++)
		mSurface[i].mD3DSurface = NULL;
	for (i = 0; i < MAX_TEXTURES; i++)
		mTexture[i].mD3DTexture = NULL;
	for (i = 0; i < 4; i++)
	{
		mQuad[i].mZ = 1.0f;
		mQuad[i].mRhw = 1.0f;
	}

	mD3DViewPort.MinZ = 0.0f;
	mD3DViewPort.MaxZ = 1.0f;

	if ((mD3DInterface = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
	{
		LOGTEXT("CDirectXRenderLayer: Direct3DCreate9() failed");
	}
}

CDirectXRenderLayer::~CDirectXRenderLayer()
{
	releaseResources();
	if (mD3DInterface)
	{
		mD3DInterface->Release();
		mD3DInterface = NULL;
	}
}

unsigned int CDirectXRenderLayer::getAdapterCount()
{
	if (mD3DInterface) return mD3DInterface->GetAdapterCount();
	return 0;
}

int CDirectXRenderLayer::getAdapterInfo(unsigned int adapter, TAdapterInfo &adapterInfo)
{
	if (mD3DInterface)
	{
		D3DFORMAT format;
		D3DADAPTER_IDENTIFIER9 identifier;
		D3DDISPLAYMODE display;
		int modeCount, i, j, adapterDisplay;
		if (FAILED(mD3DInterface->GetAdapterIdentifier(adapter, 0, &identifier)))
		{
			LOGTEXT("CDirectXRenderLayer: GetAdapterIdentifier() failed");
			return E_FAILED;
		}
		memcpy(adapterInfo.mDescription, identifier.Description, MAX_ADAPTER_DESCRIPTION_LENGTH);
		memcpy(adapterInfo.mDeviceName, identifier.DeviceName, MAX_DEVICE_NAME_LENGTH);
		adapterInfo.mDisplayModeCount = 0;
		for (i = 0; i < EFormatCount; i++)
			adapterInfo.mDisplayModeCount += mD3DInterface->GetAdapterModeCount(adapter, convertTFormatToD3DFormat((TFormat)i));
		adapterInfo.mDisplayMode = new TDisplayMode[adapterInfo.mDisplayModeCount];
		adapterDisplay = 0;
		for (i = 0; i < EFormatCount; i++)
		{
			format = convertTFormatToD3DFormat((TFormat)i);
			modeCount = mD3DInterface->GetAdapterModeCount(adapter, format);
			for (j = 0; j < modeCount; j++)
			{
				if (FAILED(mD3DInterface->EnumAdapterModes(adapter, format, j, &display)))
				{
					LOGTEXT("CDirectXRenderLayer: EnumAdapterModes() failed");
					delete [] adapterInfo.mDisplayMode;
					adapterInfo.mDisplayMode = NULL;
					adapterInfo.mDisplayModeCount = 0;
					return E_FAILED;
				}
				adapterInfo.mDisplayMode[adapterDisplay].mFormat = (TFormat)i;
				adapterInfo.mDisplayMode[adapterDisplay].mWidth = display.Width;
				adapterInfo.mDisplayMode[adapterDisplay].mHeight = display.Height;
				adapterInfo.mDisplayMode[adapterDisplay++].mRefreshRate = display.RefreshRate;
			}
		}
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::getAdapterDisplayMode(unsigned int adapter, TDisplayMode& displayMode)
{
	// stub
	return E_FAILED;
}

int CDirectXRenderLayer::validateDeviceCapabilities(unsigned int adapter, TRasterizationType rasterization)
{
	bool passed = true;
	LOGTEXT2("Validating capabilities for device#%d...", adapter);

	if (mD3DInterface)
	{
		D3DCAPS9 deviceCaps;
		D3DDEVTYPE devType = convertRasterizationTypeToDevType(rasterization);

		if (SUCCEEDED(mD3DInterface->GetDeviceCaps(adapter, devType, &deviceCaps)))
		{
			// ----------------ERRORS----------------
			if ((deviceCaps.PresentationIntervals & D3DPRESENT_INTERVAL_ONE) == 0)
			{
				LOGTEXT("    ERROR: Device does not support a presentation swap interval of every screen refresh");
				passed = false;
			}

			if ((deviceCaps.SrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) == 0)
			{
				LOGTEXT("    ERROR: Device does not support (As, As, As, As) blend factor");
				passed = false;
			}

			if ((deviceCaps.DestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) == 0)
			{
				LOGTEXT("    ERROR: Device does not support (1 - As, 1 - As, 1 - As, 1 - As) blend factor");
				passed = false;
			}

			if ((deviceCaps.TextureCaps & D3DPTEXTURECAPS_ALPHA) == 0)
			{
				LOGTEXT("    ERROR: Device does not support alpha in texture pixels");
				passed = false;
			}

			if ((deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) == 0)
			{
				LOGTEXT("    ERROR: Device does not support per-stage bilinear interpolation filtering for magnifying mipmaps");
				passed = false;
			}

			if ((deviceCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) == 0)
			{
				LOGTEXT("    ERROR: Device does not support per-stage bilinear filtering for minifying textures");
				passed = false;
			}

			if ((deviceCaps.TextureAddressCaps & D3DPTADDRESSCAPS_WRAP) == 0)
			{
				LOGTEXT("    ERROR: Device does not support wrapping of textures to addresses");
				passed = false;
			}

			if (deviceCaps.MaxTextureWidth < MIN_TEXTURE_SIZE || deviceCaps.MaxTextureHeight < MIN_TEXTURE_SIZE)
			{
				LOGTEXT3("    ERROR: Device does not support %dx%d or greater texture dimensions", MIN_TEXTURE_SIZE, MIN_TEXTURE_SIZE);
				passed = false;
			}

			if ((deviceCaps.TextureOpCaps & D3DTEXOPCAPS_MODULATE) == 0)
			{
				LOGTEXT("    ERROR: Device does not support texture-blending operation of modulation");
				passed = false;
			}

			if (deviceCaps.MaxVertexIndex < (VERTEX_BUFFER_LENGTH - 1))
			{
				LOGTEXT("    ERROR: Device does not support required max vertex index");
				passed = false;
			}

			if ((deviceCaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) == 0)
			{
				LOGTEXT("    ERROR: Device does not support bilinear interpolation filtering for minifying rectangles");
				passed = false;
			}

			if ((deviceCaps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) == 0)
			{
				LOGTEXT("    ERROR: Device does not support bilinear interpolation filtering for magnifying rectangles");
				passed = false;
			}

			if ((deviceCaps.DevCaps & D3DDEVCAPS_TEXTUREVIDEOMEMORY) == 0)
			{
				LOGTEXT("    ERROR: Device cannot retrieve textures from video memory");
				passed = false;
			}

			// ---------------WARNINGS---------------
			// add warnings here
		}
		else
		{
			LOGTEXT("    ERROR: GetDeviceCaps() failed");
			passed = false;
		}
	}
	else
	{
		LOGTEXT("    ERROR: D3DInterface is not initialized");
		passed = false;
	}

	if (passed)
	{
		LOGTEXT("Validation passed");
		return E_SUCCESS;
	}

	LOGTEXT("Validation failed");
	return E_FAILED;
}

int CDirectXRenderLayer::initDevice(unsigned int adapter, TRasterizationType rasterization, const TDisplayMode& displayMode)
{
	if (mD3DInterface)
	{
		D3DDEVTYPE devType;
		releaseResources();

		devType = convertRasterizationTypeToDevType(rasterization);

		// Filling present parameters...
		memcpy(&mDeviceDisplayMode, &displayMode, sizeof(TDisplayMode));
		mCurrentFormat = convertTFormatToD3DFormat(displayMode.mFormat);
		ZeroMemory(&mPresentParams, sizeof(mPresentParams));
		mPresentParams.Windowed = FALSE;
		mPresentParams.BackBufferWidth = displayMode.mWidth;
		mPresentParams.BackBufferHeight = displayMode.mHeight;
		mPresentParams.BackBufferCount = 1;
		mPresentParams.FullScreen_RefreshRateInHz = displayMode.mRefreshRate;
		mPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		mPresentParams.BackBufferFormat = mCurrentFormat;
		/* TODO: Try to use D3DSWAPEFFECT_FLIP for better performance.
		   Maybe store pointers to two back buffers and then swap them
		   right after presentScene(). */
		mPresentParams.SwapEffect = D3DSWAPEFFECT_COPY;
		mPresentParams.Flags = 0;

		// Creating device...
		if (FAILED(mD3DInterface->CreateDevice(adapter, devType, mHwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &mPresentParams, &mD3DDevice)))
		{
			LOGTEXT("CDirectXRenderLayer: CreateDevice() with hardware vertex processing failed");
			if (FAILED(mD3DInterface->CreateDevice(adapter, devType, mHwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &mPresentParams, &mD3DDevice)))
			{
				LOGTEXT("CDirectXRenderLayer: CreateDevice() with software vertex processing failed");
				releaseResources();
				return E_FAILED;
			}
		}

		// Retrieving device capabilities...
		if (FAILED(mD3DInterface->GetDeviceCaps(adapter, devType, &mDeviceCaps)))
		{
			LOGTEXT("CDirectXRenderLayer: GetDeviceCaps() failed");
			releaseResources();
			return E_FAILED;
		}
		mMaxPrimitiveCount = XMATH_MIN(mDeviceCaps.MaxPrimitiveCount, MAX_SPRITES_PER_TEXTURE * 2);

		// Creating font sprite...
		if (FAILED(D3DXCreateSprite(mD3DDevice, &mD3DFontSprite)))
		{
			LOGTEXT("CDirectXRenderLayer: D3DXCreateSprite() for font failed");
			releaseResources();
			return E_FAILED;
		}

		return resetDevice();
	}
	LOGTEXT("CDirectXRenderLayer: initDevice() failed");
	return E_FAILED;
}

int CDirectXRenderLayer::getDeviceDisplayMode(TDisplayMode& displayMode)
{
	if (mD3DDevice)
	{
		memcpy(&displayMode, &mDeviceDisplayMode, sizeof(TDisplayMode));
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::getMaxTextureSize(unsigned long& maxWidth, unsigned long& maxHeight)
{
	if (mD3DDevice)
	{
		maxWidth = mDeviceCaps.MaxTextureWidth;
		maxHeight = mDeviceCaps.MaxTextureHeight;
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::beginScene()
{
	if (mD3DDevice)
		return (validateDeviceStatus() && SUCCEEDED(mD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0)) && SUCCEEDED(mD3DDevice->BeginScene()));
	return E_FAILED;
}

int CDirectXRenderLayer::endScene()
{
	if (mD3DDevice)
		return SUCCEEDED(mD3DDevice->EndScene());
	return E_FAILED;
}

int CDirectXRenderLayer::presentScene()
{
	if (mD3DDevice)
		return SUCCEEDED(mD3DDevice->Present(NULL, NULL, NULL, NULL));
	return E_FAILED;
}

int CDirectXRenderLayer::setRenderTarget(SURFACE_REF surface)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		if (SUCCEEDED(mD3DDevice->SetRenderTarget(0, mSurface[surface].mD3DSurface)))
		{
			// Since now this surface will be treated as a current back buffer
			mBackBuffer = mSurface[surface].mD3DSurface;

			return E_SUCCESS;
		}
		else
		{
			LOGTEXT("CDirectXRenderLayer: SetRenderTarget() failed");
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::setRenderTarget(SURFACE_REF surface, TRect& clipBound)
{
	if (setRenderTarget(surface))
	{
		mD3DViewPort.X = clipBound.mX0;
		mD3DViewPort.Y = clipBound.mY0;
		mD3DViewPort.Width = clipBound.mX1;
		mD3DViewPort.Height = clipBound.mY1;
		return SUCCEEDED(mD3DDevice->SetViewport(&mD3DViewPort));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::setDefaultRenderTarget()
{
	if (mD3DDevice)
	{
		if (SUCCEEDED(mD3DDevice->SetRenderTarget(0, mDefaultBackBuffer)))
		{
			// Since now the initially created back buffer will be current
			mBackBuffer = mDefaultBackBuffer;

			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::addDeviceListener(IRenderDeviceListener* listener)
{
	return mDeviceListeners.add(listener);
}

int CDirectXRenderLayer::removeDeviceListener(int id)
{
	mDeviceListeners.remove(id);
	return E_SUCCESS;
}

unsigned long CDirectXRenderLayer::ARGB(int a, int r, int g, int b)
{
	return D3DCOLOR_ARGB(a, r, g, b);
}

int CDirectXRenderLayer::setFont(int width, int height, const char* faceName)
{
	if (mD3DDevice)
	{
		if (mD3DFont)
		{
			mD3DFont->Release();
			mD3DFont = NULL;
		}
		return SUCCEEDED(D3DXCreateFont(mD3DDevice, height, width, FW_NORMAL, D3DX_DEFAULT, FALSE, DEFAULT_CHARSET,
		                 OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_MODERN, faceName, &mD3DFont));
	}
	LOGTEXT("CDirectXRenderLayer: setFont() failed");
	return E_FAILED;
}

int CDirectXRenderLayer::beginText()
{
	if (mD3DFontSprite)
		return SUCCEEDED(mD3DFontSprite->Begin(D3DXSPRITE_ALPHABLEND));
	return E_FAILED;
}

int CDirectXRenderLayer::endText()
{
	if (mD3DFontSprite)
		return SUCCEEDED(mD3DFontSprite->End());
	return E_FAILED;
}

int CDirectXRenderLayer::outText(const char *text, TRect& destRect, unsigned long color)
{
	if (mD3DFont)
	{
		mRect[0].left = destRect.mX0;
		mRect[0].top = destRect.mY0;
		mRect[0].right = destRect.mX1;
		mRect[0].bottom = destRect.mY1;
		return SUCCEEDED(mD3DFont->DrawText(mD3DFontSprite, text, -1, &mRect[0], DT_WORDBREAK, color));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::createSurface(unsigned int width, unsigned int height, unsigned long flags, SURFACE_REF& surface)
{
	if (mD3DDevice)
	{
		if (findFreeSurfaceRef(surface))
		{
			mSurface[surface].mInitialRefCount = 0;
			if (flags & RENDER_FLAG_RENDERTARGET)
			{
				return SUCCEEDED(mD3DDevice->CreateRenderTarget(width, height, mCurrentFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &mSurface[surface].mD3DSurface, NULL));
			}
			else
			{
				return SUCCEEDED(mD3DDevice->CreateOffscreenPlainSurface(width, height, mCurrentFormat, convertRenderFlagToD3DPool(flags), &mSurface[surface].mD3DSurface, NULL));
			}
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::createSurfaceFromFile(char* fileName, unsigned int width, unsigned int height, TRect& srcRect, unsigned long flags, SURFACE_REF& surface)
{
	if (createSurface(width, height, flags, surface))
	{
		mRect[0].left = srcRect.mX0;
		mRect[0].top = srcRect.mY0;
		mRect[0].right = srcRect.mX1;
		mRect[0].bottom = srcRect.mY1;
		if (SUCCEEDED(D3DXLoadSurfaceFromFile(mSurface[surface].mD3DSurface, NULL, NULL, fileName, &mRect[0], D3DX_FILTER_NONE, 0, NULL)))
		{
			return E_SUCCESS;
		}
		else
		{
			releaseSurface(surface);
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::incSurfaceRef(SURFACE_REF surface)
{
	if (surface < MAX_SURFACES && (mSurface[surface].mD3DSurface))
	{
		mSurface[surface].mD3DSurface->AddRef();
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::fillSurface(SURFACE_REF surface, unsigned long color)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		return SUCCEEDED(mD3DDevice->ColorFill(mSurface[surface].mD3DSurface, NULL, color));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF surface)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		return SUCCEEDED(mD3DDevice->StretchRect(mSurface[surface].mD3DSurface, NULL, mBackBuffer, NULL, D3DTEXF_LINEAR));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF surface, long posX, long posY, TRect& srcRect)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		mPoint.x = posX;
		mPoint.y = posY;
		mRect[0].left = srcRect.mX0;
		mRect[0].top = srcRect.mY0;
		mRect[0].right = srcRect.mX1;
		mRect[0].bottom = srcRect.mY1;
		return SUCCEEDED(mD3DDevice->UpdateSurface(mSurface[surface].mD3DSurface, &mRect[0], mBackBuffer, &mPoint));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF surface, long posX, long posY)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		mPoint.x = posX;
		mPoint.y = posY;
		return SUCCEEDED(mD3DDevice->UpdateSurface(mSurface[surface].mD3DSurface, NULL, mBackBuffer, &mPoint));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF surface, TRect& destRect, TRect& srcRect)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		mRect[0].left = srcRect.mX0;
		mRect[0].top = srcRect.mY0;
		mRect[0].right = srcRect.mX1;
		mRect[0].bottom = srcRect.mY1;
		mRect[1].left = destRect.mX0;
		mRect[1].top = destRect.mY0;
		mRect[1].right = destRect.mX1;
		mRect[1].bottom = destRect.mY1;
		return SUCCEEDED(mD3DDevice->StretchRect(mSurface[surface].mD3DSurface, &mRect[0], mBackBuffer, &mRect[1], D3DTEXF_LINEAR));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF surface, TRect& srcRect)
{
	if ((mD3DDevice) && (surface < MAX_SURFACES))
	{
		mRect[0].left = srcRect.mX0;
		mRect[0].top = srcRect.mY0;
		mRect[0].right = srcRect.mX1;
		mRect[0].bottom = srcRect.mY1;
		return SUCCEEDED(mD3DDevice->StretchRect(mSurface[surface].mD3DSurface, &mRect[0], mBackBuffer, NULL, D3DTEXF_LINEAR));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface)
{
	if ((mD3DDevice) && (srcSurface < MAX_SURFACES) && (destSurface < MAX_SURFACES))
	{
		if (FAILED(mD3DDevice->UpdateSurface(mSurface[srcSurface].mD3DSurface, NULL, mSurface[destSurface].mD3DSurface, NULL)))
		{
			return SUCCEEDED(mD3DDevice->GetRenderTargetData(mSurface[srcSurface].mD3DSurface, mSurface[destSurface].mD3DSurface));
		}
		else
		{
			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY, TRect& srcRect)
{
	if ((mD3DDevice) && (srcSurface < MAX_SURFACES) && (destSurface < MAX_SURFACES))
	{
		mPoint.x = posX;
		mPoint.y = posY;
		mRect[0].left = srcRect.mX0;
		mRect[0].top = srcRect.mY0;
		mRect[0].right = srcRect.mX1;
		mRect[0].bottom = srcRect.mY1;
		return SUCCEEDED(mD3DDevice->UpdateSurface(mSurface[srcSurface].mD3DSurface, &mRect[0], mSurface[destSurface].mD3DSurface, &mPoint));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::drawSurface(SURFACE_REF srcSurface, SURFACE_REF destSurface, long posX, long posY)
{
	if ((mD3DDevice) && (srcSurface < MAX_SURFACES) && (destSurface < MAX_SURFACES))
	{
		mPoint.x = posX;
		mPoint.y = posY;
		return SUCCEEDED(mD3DDevice->UpdateSurface(mSurface[srcSurface].mD3DSurface, NULL, mSurface[destSurface].mD3DSurface, &mPoint));
	}
	return E_FAILED;
}	

int CDirectXRenderLayer::releaseSurface(SURFACE_REF surface)
{
	if ((surface < MAX_SURFACES) && (mSurface[surface].mD3DSurface))
	{
		if (mSurface[surface].mD3DSurface->Release() == mSurface[surface].mInitialRefCount) mSurface[surface].mD3DSurface = NULL;
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::createTexture(unsigned int width, unsigned int height, unsigned long flags, TEXTURE_REF& texture)
{
	if (mD3DDevice)
	{
		if (findFreeTextureRef(texture))
		{
			// TODO: Use texture dimensions recieved from D3DXCreateTexture() function instead
			mTexture[texture].mInvWidth = 1.0f / width;
			mTexture[texture].mInvHeight = 1.0f / height;

			return SUCCEEDED(D3DXCreateTexture(mD3DDevice, width, height, 1, convertRenderFlagToD3DUsage(flags),
			       mCurrentFormat, convertRenderFlagToD3DPool(flags), &mTexture[texture].mD3DTexture));
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::createTextureFromFile(char* fileName, unsigned long colorKey, unsigned long flags, unsigned int& width, unsigned int& height, TEXTURE_REF& texture)
{
	if (mD3DDevice)
	{
		if (findFreeTextureRef(texture))
		{
			D3DXIMAGE_INFO imageInfo;
			if (SUCCEEDED(D3DXCreateTextureFromFileEx(mD3DDevice, fileName, 0, 0, 1, convertRenderFlagToD3DUsage(flags),
				mCurrentFormat, convertRenderFlagToD3DPool(flags), D3DX_FILTER_NONE, D3DX_FILTER_NONE,
				colorKey, &imageInfo, NULL, &mTexture[texture].mD3DTexture)))
			{
				width = imageInfo.Width;
				height = imageInfo.Height;

				mTexture[texture].mInvWidth = 1.0f / width;
				mTexture[texture].mInvHeight = 1.0f / height;

				return E_SUCCESS;
			}
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::incTextureRef(TEXTURE_REF texture)
{
	if (texture < MAX_TEXTURES && (mTexture[texture].mD3DTexture))
	{
		mTexture[texture].mD3DTexture->AddRef();
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::beginSprite()
{
	if (mVertexBuffer)
	{
		return SUCCEEDED(mVertexBuffer->Lock(0, 0, (void **)&mVertexData, 0));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::endSprite()
{
	if (mVertexBuffer)
	{
		if (SUCCEEDED(mVertexBuffer->Unlock()) && (mPrimitiveCount > 0))
		{
			mD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0,
			                                 mPrimitiveCount << 1, 0, mPrimitiveCount);
			mPrimitiveCount = 0;

			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::getSpriteScale(CVector2D& scale)
{
	scale = mSpriteScale;
	return E_SUCCESS;
}

int CDirectXRenderLayer::setSpriteScale(CVector2D& scale)
{
	mSpriteScale = scale;
	return E_SUCCESS;
}

int CDirectXRenderLayer::drawSprite(TEXTURE_REF texture, const TSpriteDef& spriteDef)
{
	if ((mVertexBuffer) && (texture < MAX_TEXTURES))
	{
		if (mSpriteTextureRef != texture || mPrimitiveCount == mMaxPrimitiveCount)
		{
			endSprite();
			mD3DDevice->SetTexture(0, mTexture[texture].mD3DTexture);
			mSpriteTextureRef = texture;
			beginSprite();
		}

		UINT startVertex = mPrimitiveCount << 1;

		float invW = mTexture[texture].mInvWidth;
		float invH = mTexture[texture].mInvHeight;

		// Scaling + Translation
		float positionX, positionY;
		float width, height;

		positionX = spriteDef.mLeftTop.mX * mSpriteScale.mX;
		positionY = spriteDef.mLeftTop.mY * mSpriteScale.mY;

		width = spriteDef.mWidth * mSpriteScale.mX;
		height = spriteDef.mHeight * mSpriteScale.mY;

		if (spriteDef.mTexelToPixel)
		{
			positionX = (float)xmath_round(positionX);
			positionY = (float)xmath_round(positionY);

			width = (float)xmath_round(width);
			height = (float)xmath_round(height);
		}

		// Substract half-pixel for proper Texel-To-Pixel convertion
		positionX -= 0.5f;
		positionY -= 0.5f;

		mQuad[0].mX = positionX;
		mQuad[0].mY = positionY;
		mQuad[0].mU = spriteDef.mTextureRect.mX0 * invW;
		mQuad[0].mV = spriteDef.mTextureRect.mY0 * invH;

		mQuad[1].mX = positionX + width;
		mQuad[1].mY = mQuad[0].mY;
		mQuad[1].mU = spriteDef.mTextureRect.mX1 * invW;
		mQuad[1].mV = mQuad[0].mV;

		mQuad[2].mX = mQuad[0].mX;
		mQuad[2].mY = positionY + height;
		mQuad[2].mU = mQuad[0].mU;
		mQuad[2].mV = spriteDef.mTextureRect.mY1 * invH;

		mQuad[3].mX = mQuad[1].mX;
		mQuad[3].mY = mQuad[2].mY;
		mQuad[3].mU = mQuad[1].mU;
		mQuad[3].mV = mQuad[2].mV;

		float subX, subY;
		float centerX, centerY;

		centerX = (mQuad[0].mX + mQuad[1].mX) * 0.5f;
		centerY = (mQuad[0].mY + mQuad[2].mY) * 0.5f;

		CVector2D& dir = CVector2D::getUnitVector(spriteDef.mAngle);

		// Rotation
		for (int index = 0; index < 4; index++)
		{
			subX = mQuad[index].mX - centerX;
			subY = mQuad[index].mY - centerY;

			mQuad[index].mX = (subX * dir.mX - subY * dir.mY + centerX);
			mQuad[index].mY = (subX * dir.mY + subY * dir.mX + centerY);

			mQuad[index].mDiffuse = spriteDef.mDiffuse;
		}

		memcpy(mVertexData + startVertex, mQuad, 4 * sizeof(TVertexData));

		mPrimitiveCount += 2;

		return E_SUCCESS;
	}

	return E_FAILED;
}

int CDirectXRenderLayer::drawTexture(TEXTURE_REF srcTexture, TEXTURE_REF destTexture)
{
	if ((mD3DDevice) && (srcTexture < MAX_TEXTURES) && (destTexture < MAX_TEXTURES))
	{
		return SUCCEEDED(mD3DDevice->UpdateTexture(mTexture[srcTexture].mD3DTexture, mTexture[destTexture].mD3DTexture));
	}
	return E_FAILED;
}

int CDirectXRenderLayer::getSurfaceFromTexture(TEXTURE_REF texture, SURFACE_REF& surface)
{
	if ((mD3DDevice) && (texture < MAX_TEXTURES))
	{
		if (findFreeSurfaceRef(surface))
		{
			if (SUCCEEDED(mTexture[texture].mD3DTexture->GetSurfaceLevel(0, &mSurface[surface].mD3DSurface)))
			{
				mSurface[surface].mD3DSurface->AddRef();
				mSurface[surface].mInitialRefCount = mSurface[surface].mD3DSurface->Release() - 1;

				return E_SUCCESS;
			}
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::releaseTexture(TEXTURE_REF texture)
{
	if ((texture < MAX_TEXTURES) && (mTexture[texture].mD3DTexture))
	{
		if (mTexture[texture].mD3DTexture->Release() == 0)
		{
			mTexture[texture].mD3DTexture = NULL;
			if (mSpriteTextureRef == texture) mSpriteTextureRef = MAX_TEXTURES;
		}
		return E_SUCCESS;
	}
	return E_FAILED;
}

int CDirectXRenderLayer::beginPrimitive()
{
	// do nothing
	return E_SUCCESS;
}

int CDirectXRenderLayer::endPrimitive()
{
	// do nothing
	return E_SUCCESS;
}

int CDirectXRenderLayer::drawPolygon(TEXTURE_REF texture, TVertex* vertices, unsigned int vertexCount)
{
	if ((mVertexBuffer) && (texture < MAX_TEXTURES) && (vertices) && (vertexCount <= MAX_VERTEX_COUNT))
	{
		if (SUCCEEDED(mVertexBuffer->Lock(0, 0, (void **)&mVertexData, vertexCount * sizeof(TVertexData))))
		{
			for (unsigned int i = 0; i < vertexCount; i++)
			{
				mVertexData[i].mX = vertices[i].mX;
				mVertexData[i].mY = vertices[i].mY;
				mVertexData[i].mZ = 1.0f;
				mVertexData[i].mRhw = 1.0f;
				mVertexData[i].mDiffuse = D3DCOLOR_ARGB(255, 255, 255, 255);
				mVertexData[i].mU = vertices[i].mU;
				mVertexData[i].mV = vertices[i].mV;
			}

			if (mSpriteTextureRef != texture)
			{
				mD3DDevice->SetTexture(0, mTexture[texture].mD3DTexture);
				mSpriteTextureRef = texture;
			}

			// TODO: perform rendering in accordance with mMaxPrimitiveCount value
			return (SUCCEEDED(mVertexBuffer->Unlock()) && SUCCEEDED(mD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, vertexCount - 2)));
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::release()
{
	mRefCount--;

	if (mRefCount == 0)
	{
		delete mInstance;
		mInstance = NULL;
	}

	return E_SUCCESS;
}

void CDirectXRenderLayer::releaseResources()
{
	unsigned int i;
	for (i = 0; i < MAX_SURFACES; i++)
		if (mSurface[i].mD3DSurface) {
			while (mSurface[i].mD3DSurface->Release() > mSurface[i].mInitialRefCount);
			mSurface[i].mD3DSurface = NULL;
		}
	for (i = 0; i < MAX_TEXTURES; i++)
		if (mTexture[i].mD3DTexture) {
			while (mTexture[i].mD3DTexture->Release() > 0);
			mTexture[i].mD3DTexture = NULL;
		}
	if (mD3DFont)
	{
		mD3DFont->Release();
		mD3DFont = NULL;
	}
	if (mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = NULL;
	}
	if (mVertexBuffer)
	{
		mVertexBuffer->Release();
		mVertexBuffer = NULL;
	}
	if (mDefaultBackBuffer)
	{
		mDefaultBackBuffer->Release();
		mDefaultBackBuffer = NULL;
	}
	mBackBuffer = NULL;
	if (mD3DFontSprite)
	{
		mD3DFontSprite->Release();
		mD3DFontSprite = NULL;
	}
	if (mD3DDevice)
	{
		mD3DDevice->Release();
		mD3DDevice = NULL;
	}
}

D3DFORMAT CDirectXRenderLayer::convertTFormatToD3DFormat(TFormat format)
{
	D3DFORMAT d3dFormat;
	switch (format)
	{
		case EFormatR3G3B2: d3dFormat = D3DFMT_R3G3B2; break;
		case EFormatX1R5G5B5: d3dFormat = D3DFMT_X1R5G5B5; break;
		case EFormatA1R5G5B5: d3dFormat = D3DFMT_A1R5G5B5; break;
		case EFormatA4R4G4B4: d3dFormat = D3DFMT_A4R4G4B4; break;
		case EFormatR5G6B5: d3dFormat = D3DFMT_R5G6B5; break;
		case EFormatX8R8G8B8: d3dFormat = D3DFMT_X8R8G8B8; break;
		case EFormatA8R8G8B8: d3dFormat = D3DFMT_A8R8G8B8; break;
		default: d3dFormat = D3DFMT_UNKNOWN; break;
	}
	return d3dFormat;
}

D3DPOOL CDirectXRenderLayer::convertRenderFlagToD3DPool(unsigned long flags)
{
	D3DPOOL pool;
	if (flags & RENDER_FLAG_VIDEOMEM)
	{
		pool = D3DPOOL_DEFAULT;
	}
	else if (flags & RENDER_FLAG_SYSTEMMEM)
	{
		pool = D3DPOOL_SYSTEMMEM;
	}
	else if (flags & RENDER_FLAG_MANAGEDMEM)
	{
		pool = D3DPOOL_MANAGED;
	}
	else
	{
		pool = D3DPOOL_DEFAULT;
	}
	return pool;
}

unsigned long CDirectXRenderLayer::convertRenderFlagToD3DUsage(unsigned long flags)
{
	unsigned long usage = 0;
	if (flags & RENDER_FLAG_RENDERTARGET)
	{
		usage = D3DUSAGE_RENDERTARGET;
	}
	return usage;
}

D3DDEVTYPE CDirectXRenderLayer::convertRasterizationTypeToDevType(TRasterizationType rasterization)
{
	D3DDEVTYPE devType;

	switch (rasterization)
	{
		case ERasterizationTypeSoftware:
		{
			devType = D3DDEVTYPE_REF;
		} break;
		case ERasterizationTypeHardware:
		{
			devType = D3DDEVTYPE_HAL;
		} break;
		default:
		{
			LOGTEXT("CDirectXRenderLayer: unknown rasterization type, defaulting to HAL");
			devType = D3DDEVTYPE_HAL;
		} break;
	}

	return devType;
}

int CDirectXRenderLayer::findFreeSurfaceRef(SURFACE_REF& ref)
{
	for (ref = 0; ref < MAX_SURFACES; ref++)
		if (mSurface[ref].mD3DSurface == NULL) return E_SUCCESS;
	return E_FAILED;
}

int CDirectXRenderLayer::findFreeTextureRef(TEXTURE_REF& ref)
{
	for (ref = 0; ref < MAX_TEXTURES; ref++)
		if (mTexture[ref].mD3DTexture == NULL) return E_SUCCESS;
	return E_FAILED;
}

int CDirectXRenderLayer::validateDeviceStatus()
{
	if (mD3DDevice)
	{
		HRESULT hr = mD3DDevice->TestCooperativeLevel();

		if (hr == D3DERR_DEVICELOST && !mIsDeviceLostHandled)
		{
			mD3DDevice->EvictManagedResources();
			mD3DDevice->SetIndices(NULL);
			mD3DDevice->SetStreamSource(0, NULL, 0, sizeof(TVertexData));

			mIndexBuffer->Release();
			mIndexBuffer = NULL;

			mVertexBuffer->Release();
			mVertexBuffer = NULL;

			mDefaultBackBuffer->Release();
			mDefaultBackBuffer = mBackBuffer = NULL;

			if (mD3DFont) mD3DFont->OnLostDevice();
			mD3DFontSprite->OnLostDevice();

			notifyOnDeviceLost();
			mIsDeviceLostHandled = true;
		}
		else if (hr == D3DERR_DEVICENOTRESET)
		{
			if (SUCCEEDED(hr = mD3DDevice->Reset(&mPresentParams)))
			{
				if (mD3DFont) mD3DFont->OnResetDevice();
				mD3DFontSprite->OnResetDevice();

				resetDevice();

				notifyOnDeviceReset();
			}
			else
			{
				LOGTEXT("CDirectXRenderLayer: device reset failed");
			}
		}
		else
		{
			return E_SUCCESS;
		}
	}
	return E_FAILED;
}

int CDirectXRenderLayer::resetDevice()
{
	if (mD3DDevice)
	{
		// Setting render states for created device...
		mD3DDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		mD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		mD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		mD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		mD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		mD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		// Enabling alpha test if possible
		if (mDeviceCaps.AlphaCmpCaps & (D3DPCMPCAPS_GREATER | D3DPCMPCAPS_GREATEREQUAL))
		{
			DWORD alphaRef = 1;
			DWORD alphaFunc = D3DCMP_GREATEREQUAL;

			if (mDeviceCaps.AlphaCmpCaps & D3DPCMPCAPS_GREATER)
			{
				alphaRef = 0;
				alphaFunc = D3DCMP_GREATER;
			}

			mD3DDevice->SetRenderState(D3DRS_ALPHAREF, alphaRef);
			mD3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
			mD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, alphaFunc);
		}

		// Setting texture stage states for created device...
		mD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		mD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		mD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		mD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		mD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		mD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		mD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		mD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		// Getting default back buffer...
		if (FAILED(mD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &mDefaultBackBuffer)))
		{
			LOGTEXT("CDirectXRenderLayer: GetBackBuffer() failed");
			releaseResources();
			return E_FAILED;
		}

		// Initially current back buffer will be default
		mBackBuffer = mDefaultBackBuffer;

		// Creating vertex buffer...
		if (FAILED(mD3DDevice->CreateVertexBuffer(VERTEX_BUFFER_LENGTH * sizeof(TVertexData), D3DUSAGE_WRITEONLY, VERTEX_BUFFER_FVF, D3DPOOL_DEFAULT, &mVertexBuffer, NULL)))
		{
			LOGTEXT("CDirectXRenderLayer: CreateVertexBuffer() failed");
			releaseResources();
			return E_FAILED;
		}

		// Creating index buffer...
#ifdef USE_32BIT_INDEX_BUFFER
		const D3DFORMAT indexBufferFormat = D3DFMT_INDEX32;
		const DWORD indexSize = sizeof(DWORD);
#else
		const D3DFORMAT indexBufferFormat = D3DFMT_INDEX16;
		const DWORD indexSize = sizeof(WORD);
#endif
		if (FAILED(mD3DDevice->CreateIndexBuffer(MAX_SPRITES_PER_TEXTURE * 6 * indexSize, D3DUSAGE_WRITEONLY, indexBufferFormat, D3DPOOL_DEFAULT, &mIndexBuffer, NULL)))
		{
			LOGTEXT("CDirectXRenderLayer: CreateIndexBuffer() failed");
			releaseResources();
			return E_FAILED;
		}

		// Filling index buffer...
#ifdef USE_32BIT_INDEX_BUFFER
		DWORD *indices, index = 0;
#else
		WORD *indices, index = 0;
#endif
		if (FAILED(mIndexBuffer->Lock(0, 0, (void **)&indices, 0)))
		{
			LOGTEXT("CDirectXRenderLayer: failed to lock index buffer");
			releaseResources();
			return E_FAILED;
		}
		for (int i = 0; i < MAX_SPRITES_PER_TEXTURE; i++)
		{
			*indices++ = index;
			*indices++ = index + 1;
			*indices++ = index + 2;
			*indices++ = index + 2;
			*indices++ = index + 3;
			*indices++ = index + 1;

			index += 4;
		}
		mIndexBuffer->Unlock();

		// Setting index buffer...
		if (FAILED(mD3DDevice->SetIndices(mIndexBuffer)))
		{
			LOGTEXT("CDirectXRenderLayer: SetIndices() failed");
			releaseResources();
			return E_FAILED;
		}

		// Setting FVF...
		if (FAILED(mD3DDevice->SetFVF(VERTEX_BUFFER_FVF)))
		{
			LOGTEXT("CDirectXRenderLayer: SetFVF() failed");
			releaseResources();
			return E_FAILED;
		}

		// Setting stream source...
		if (FAILED(mD3DDevice->SetStreamSource(0, mVertexBuffer, 0, sizeof(TVertexData))))
		{
			LOGTEXT("CDirectXRenderLayer: SetStreamSource() failed");
			releaseResources();
			return E_FAILED;
		}

		mSpriteTextureRef = MAX_TEXTURES;
		mIsDeviceLostHandled = false;

		return E_SUCCESS;
	}

	return E_FAILED;
}

void CDirectXRenderLayer::notifyOnDeviceLost()
{
	IRenderDeviceListener* listener;

	mDeviceListeners.reset();
	while ((listener = mDeviceListeners.next()) != NULL) listener->onRenderDeviceLost();
}

void CDirectXRenderLayer::notifyOnDeviceReset()
{
	IRenderDeviceListener* listener;

	mDeviceListeners.reset();
	while ((listener = mDeviceListeners.next()) != NULL) listener->onRenderDeviceReset();
}

// ==========================================================

IRenderLayer* obtainDirectXRenderLayer(HWND hwnd)
{
	return CDirectXRenderLayer::obtainInstance(hwnd);
}