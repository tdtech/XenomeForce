#pragma once

#include "..\box2d\Box2D.h"
#include "..\common\collection.h"
#include "physics_layer.h"

struct TBodyHolder
{
	TBodyHolder(b2Body* body, TBodyHandler* handler):
	mBody(body),
	mHandler(handler),
	mDynamicId(INVALID_ID)
	{}

	b2Body*       mBody;
	TBodyHandler* mHandler;
	int           mDynamicId; // for dynamic body only
};

class CBox2DPhysicsLayer: public IPhysicsLayer,
                          public b2ContactListener,
                          public b2RayCastCallback,
                          public b2QueryCallback
{
public:
	static CBox2DPhysicsLayer* obtainInstance();

	int setTimeStep(float timeStep);
	void setGravity(CVector2D& gravity);

	int createRectangleBody(TBodyDef& bodyDef, float width, float height, TBodyHandler* handler, BODY_REF& body);
	int createCircleBody(TBodyDef& bodyDef, float radius, TBodyHandler* handler, BODY_REF& body);
	int createPolygonBody(TBodyDef& bodyDef, CVector2D* vertices, int count, TBodyHandler* handler, BODY_REF& body);
	int createChainBody(TBodyDef& bodyDef, CVector2D* chain, int count, bool loop, TBodyHandler* handler, BODY_REF& body);

	void setActive(BODY_REF body, bool active);
	void setVelocity(BODY_REF body, CVector2D& velocity);
	void setImpulse(BODY_REF body, CVector2D& point, CVector2D& impulse);
	void setTransform(BODY_REF body, CVector2D& position, float angle);
	void getPosition(BODY_REF body, CVector2D& position);
	float getAngle(BODY_REF body);
	void getDistance(BODY_REF body, CVector2D& point, TBodyDistance& distance);
	bool rayCast(CVector2D& point1, CVector2D& point2, unsigned int filterBits, TRayCastResults& results);
	void searchBodies(CVector2D& position, float radius, unsigned int filterBits);
	TBodySearchResults* getBodySearchResults();

	void destroyBody(BODY_REF body);

	void setContactListener(IContactListener& listener);

	void simulate();
	bool isSimulating();

	void destroyAllBodies();

	int release();

	// b2ContactListener implementation
	void BeginContact(b2Contact* contact);
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);

	// b2RayCastCallback implementation
	float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);

	// b2QueryCallback implementation
	bool ReportFixture(b2Fixture* fixture);

private:
	CBox2DPhysicsLayer();
	~CBox2DPhysicsLayer();

	b2Body* createInternalBody(TBodyDef& bodyDef);
	void destroyBodyHolder(TBodyHolder* holder);

private:
	CCollection<TBodyHolder>   mBodies;
	CCollection<TBodyHolder>   mDynamicBodies;

	float                      mTimeStep;
	float                      mInverseTimeStep;

	IContactListener&          mContactListener;

	b2SimplexCache             mSimplexCache;
	b2DistanceInput            mDistanceInput;
	b2DistanceOutput           mDistanceOutput;
	b2CircleShape              mEpsilonShape;

	// for ray-cast
	b2Fixture*                 mReportedFixture;
	b2Vec2                     mInitialIntersection;

	TBodySearchResults*        mBodySearchResults;
	int                        mBodySearchResultsCapacity;
	b2Vec2                     mSearchCenter;
	float                      mSearchRadius;

	unsigned int               mFilterBits; // used by ray-cast and body-search callbacks

	b2World*                   mWorld;

	static b2Vec2              mZeroVelocity;

	static CBox2DPhysicsLayer* mInstance;
	static int                 mRefCount;
};