#pragma once

#include "..\common\xmath.h"

typedef int BODY_REF;

enum TBodyType
{
	 EStaticBody
	,EDynamicBody
};

struct TBodyDef
{
	TBodyDef()
	{
		mAngle = 0.0f;
		mFrameRadius = 0.0f;
		mType = EStaticBody;
		mIsActive = true;
		mIsBullet = false;
		mIsFixedRotation = false;
		mIsSensor = false;
		mMass = 1.0f;
		mFriction = 0.0f;
		mRestitution = 0.0f;
		mDeceleration = 0.0f;
		mAngularDeceleration = 0.0f;
		mCategoryBits = 0xFFFF;
		mFilterBits = 0xFFFF;
		mUserData = 0;
	}

	CVector2D    mPosition;
	float        mAngle;
	float        mFrameRadius; // used when searching bodies

	TBodyType    mType;

	bool         mIsActive;
	bool         mIsBullet;
	bool         mIsFixedRotation; // Should this body be prevented from rotating? Useful for characters
	bool         mIsSensor;

	float        mMass; // in kg
	float        mFriction;
	float        mRestitution;
	float        mDeceleration;
	float        mAngularDeceleration;

	unsigned int mCategoryBits;
	unsigned int mFilterBits;

	void*        mUserData;
	// add more definitions...
};

// Handler is mandatory only for dynamic bodies
struct TBodyHandler
{
	CVector2D mPosition;
	// add more members...
};

struct TBodyDistance
{
	CVector2D mClosestPoint;
	float     mDistance;
};

struct TRayCastResults
{
	void*     mUserData; // user data of closest body
	CVector2D mIntersection; // point of initial intersection
};

struct TBodySearchResults
{
	void** mUserData;
	int    mBodyCount;
};

// Contact solutions
#define CONTACT_SOLUTION_CONTINUE    0x0000
#define CONTACT_SOLUTION_IGNORE      0x0001
#define CONTACT_SOLUTION_STOP_BODY_A 0x0002
#define CONTACT_SOLUTION_STOP_BODY_B 0x0004

class IContactListener
{
public:
	virtual ~IContactListener() {}

	virtual unsigned int onContact(void* userDataA, void* userDataB) = 0; // return the combination of contact solution flags
	virtual void onSensorTouched(void* sensorUserData, void* bodyUserData) = 0;
};

class IPhysicsLayer
{
public:
	virtual int setTimeStep(float timeStep) = 0; // in seconds
	virtual void setGravity(CVector2D& gravity) = 0;

	virtual int createRectangleBody(TBodyDef& bodyDef, float width, float height, TBodyHandler* handler, BODY_REF& body) = 0;
	virtual int createCircleBody(TBodyDef& bodyDef, float radius, TBodyHandler* handler, BODY_REF& body) = 0;
	virtual int createPolygonBody(TBodyDef& bodyDef, CVector2D* vertices, int count, TBodyHandler* handler, BODY_REF& body) = 0;
	virtual int createChainBody(TBodyDef& bodyDef, CVector2D* chain, int count, bool loop, TBodyHandler* handler, BODY_REF& body) = 0;

	virtual void setActive(BODY_REF body, bool active) = 0;
	virtual void setVelocity(BODY_REF body, CVector2D& velocity) = 0;
	virtual void setImpulse(BODY_REF body, CVector2D& point, CVector2D& impulse) = 0;
	virtual void setTransform(BODY_REF body, CVector2D& position, float angle) = 0;
	virtual void getPosition(BODY_REF body, CVector2D& position) = 0;
	virtual float getAngle(BODY_REF body) = 0; // in degrees
	virtual void getDistance(BODY_REF body, CVector2D& point, TBodyDistance& distance) = 0; // chain body is not supported
	virtual bool rayCast(CVector2D& point1, CVector2D& point2, unsigned int filterBits, TRayCastResults& results) = 0;
	virtual void searchBodies(CVector2D& position, float radius, unsigned int filterBits) = 0;
	virtual TBodySearchResults* getBodySearchResults() = 0;

	virtual void destroyBody(BODY_REF body) = 0;

	virtual void setContactListener(IContactListener& listener) = 0;

	virtual void simulate() = 0;
	virtual bool isSimulating() = 0;

	virtual void destroyAllBodies() = 0;

	virtual int release() = 0;

protected:
	virtual ~IPhysicsLayer() {}
};

// ==========================================================

// Methods to obtain physics layers
IPhysicsLayer* obtainBox2DPhysicsLayer();