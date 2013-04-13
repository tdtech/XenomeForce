#include "..\common\common.h"
#include "box2d_physics_layer.h"

struct TBodyUserData
{
	float mFrameRadius;
};

class CDefaultContactListener: public IContactListener
{
public:
	CDefaultContactListener();
	~CDefaultContactListener();

	unsigned int onContact(void* userDataA, void* userDataB);
	void onSensorTouched(void* sensorUserData, void* bodyUserData);
};

CDefaultContactListener::CDefaultContactListener() {}

CDefaultContactListener::~CDefaultContactListener() {}

unsigned int CDefaultContactListener::onContact(void* userDataA, void* userDataB)
{
	return CONTACT_SOLUTION_CONTINUE;
}

void CDefaultContactListener::onSensorTouched(void* sensorUserData, void* bodyUserData) {}

// Internal default contact listener
CDefaultContactListener sDefaultContactListener;

//-----------------------------------------------

#define BODIES_INITIAL_CAPACITY 1024

#define PIXELS_TO_METERS(x) (x * 0.03125f)
#define METERS_TO_PIXELS(x) (x * 32.0f)

#define VELOCITY_ITERATIONS 8
#define POSITION_ITERATIONS 4

CBox2DPhysicsLayer* CBox2DPhysicsLayer::mInstance = 0;
int                 CBox2DPhysicsLayer::mRefCount = 0;
b2Vec2              CBox2DPhysicsLayer::mZeroVelocity(0.0f, 0.0f);

CBox2DPhysicsLayer* CBox2DPhysicsLayer::obtainInstance()
{
	if (mInstance == 0) mInstance = new CBox2DPhysicsLayer();

	mRefCount++;
	return mInstance;
}

CBox2DPhysicsLayer::CBox2DPhysicsLayer():
mBodies(BODIES_INITIAL_CAPACITY),
mDynamicBodies(BODIES_INITIAL_CAPACITY),
mContactListener(sDefaultContactListener)
{
	b2Vec2 gravity(0.0f, 0.0f);
	mWorld = new b2World(gravity);
	mWorld->SetContactListener(this);

	mEpsilonShape.m_radius = b2_epsilon;

	mDistanceInput.proxyA.Set(&mEpsilonShape, 0);
	mDistanceInput.transformA.SetIdentity();

	setTimeStep(1.0f / 60.0f);

	mBodySearchResultsCapacity = BODIES_INITIAL_CAPACITY;
	mBodySearchResults = new TBodySearchResults;
	mBodySearchResults->mUserData = new void*[mBodySearchResultsCapacity];
	mBodySearchResults->mBodyCount = 0;
}

CBox2DPhysicsLayer::~CBox2DPhysicsLayer()
{
	delete mWorld;
}

int CBox2DPhysicsLayer::setTimeStep(float timeStep)
{
	if (timeStep > 0)
	{
		mTimeStep = timeStep;
		mInverseTimeStep = 1 / timeStep;
		return E_SUCCESS;
	}
	return E_FAILED;
}

void CBox2DPhysicsLayer::setGravity(CVector2D& gravity)
{
	b2Vec2 internalGravity(PIXELS_TO_METERS(gravity.mX), PIXELS_TO_METERS(gravity.mY));
	mWorld->SetGravity(internalGravity);
}

int CBox2DPhysicsLayer::createRectangleBody(TBodyDef& bodyDef, float width, float height, TBodyHandler* handler, BODY_REF& body)
{
	b2Body* internalBody = createInternalBody(bodyDef);

	float internalWidth = PIXELS_TO_METERS(width);
	float internalHeight = PIXELS_TO_METERS(height);

	// Attaching rectangle shape
	b2PolygonShape rectangle;
	rectangle.SetAsBox(internalWidth * 0.5f, internalHeight * 0.5f);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectangle;
	fixtureDef.userData = bodyDef.mUserData;
	fixtureDef.friction = bodyDef.mFriction;
	fixtureDef.restitution = bodyDef.mRestitution;
	fixtureDef.density = bodyDef.mMass / (internalWidth * internalHeight);
	fixtureDef.isSensor = bodyDef.mIsSensor;
	fixtureDef.filter.categoryBits = bodyDef.mCategoryBits;
	fixtureDef.filter.maskBits = bodyDef.mFilterBits;
	internalBody->CreateFixture(&fixtureDef);

	if (handler) handler->mPosition = bodyDef.mPosition;

	TBodyHolder* holder = new TBodyHolder(internalBody, handler);
	body = mBodies.add(holder);

	if ((bodyDef.mType == EDynamicBody) && bodyDef.mIsActive)
	{
		holder->mDynamicId = mDynamicBodies.add(holder);
	}

	return E_SUCCESS;
}

int CBox2DPhysicsLayer::createCircleBody(TBodyDef& bodyDef, float radius, TBodyHandler* handler, BODY_REF& body)
{
	b2Body* internalBody = createInternalBody(bodyDef);

	float internalRadius = PIXELS_TO_METERS(radius);

	// Attaching circle shape
	b2CircleShape circle;
	circle.m_radius = internalRadius;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circle;
	fixtureDef.userData = bodyDef.mUserData;
	fixtureDef.friction = bodyDef.mFriction;
	fixtureDef.restitution = bodyDef.mRestitution;
	fixtureDef.density = bodyDef.mMass / (b2_pi * internalRadius * internalRadius);
	fixtureDef.isSensor = bodyDef.mIsSensor;
	fixtureDef.filter.categoryBits = bodyDef.mCategoryBits;
	fixtureDef.filter.maskBits = bodyDef.mFilterBits;
	internalBody->CreateFixture(&fixtureDef);

	if (handler) handler->mPosition = bodyDef.mPosition;

	TBodyHolder* holder = new TBodyHolder(internalBody, handler);
	body = mBodies.add(holder);

	if ((bodyDef.mType == EDynamicBody) && bodyDef.mIsActive)
	{
		holder->mDynamicId = mDynamicBodies.add(holder);
	}

	return E_SUCCESS;
}

int CBox2DPhysicsLayer::createPolygonBody(TBodyDef& bodyDef, CVector2D* vertices, int count, TBodyHandler* handler, BODY_REF& body)
{
	b2Body* internalBody = createInternalBody(bodyDef);

	b2Vec2 s(0.0f, 0.0f);
	b2Vec2* internalVertices = new b2Vec2[count];
	for (int index = 0; index < count; index++)
	{
		internalVertices[index].x = PIXELS_TO_METERS(vertices[index].mX);
		internalVertices[index].y = PIXELS_TO_METERS(vertices[index].mY);

		s += internalVertices[index];
	}
	s *= 1.0f / count;

	// Compute polygon area
	float area = 0.0f;
	for (int index = 0; index < count; index++)
	{
		b2Vec2 e1 = internalVertices[index] - s;
		b2Vec2 e2 = index + 1 < count ? internalVertices[index + 1] - s : internalVertices[0] - s;

		area += 0.5f * b2Cross(e1, e2);
	}

	// Attaching polygon shape
	b2PolygonShape polygon;
	polygon.Set(internalVertices, count);
	delete [] internalVertices;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polygon;
	fixtureDef.userData = bodyDef.mUserData;
	fixtureDef.friction = bodyDef.mFriction;
	fixtureDef.restitution = bodyDef.mRestitution;
	fixtureDef.density = bodyDef.mMass / area;
	fixtureDef.isSensor = bodyDef.mIsSensor;
	fixtureDef.filter.categoryBits = bodyDef.mCategoryBits;
	fixtureDef.filter.maskBits = bodyDef.mFilterBits;
	internalBody->CreateFixture(&fixtureDef);

	if (handler) handler->mPosition = bodyDef.mPosition;

	TBodyHolder* holder = new TBodyHolder(internalBody, handler);
	body = mBodies.add(holder);

	if ((bodyDef.mType == EDynamicBody) && bodyDef.mIsActive)
	{
		holder->mDynamicId = mDynamicBodies.add(holder);
	}

	return E_SUCCESS;
}

int CBox2DPhysicsLayer::createChainBody(TBodyDef& bodyDef, CVector2D* chain, int count, bool loop, TBodyHandler* handler, BODY_REF& body)
{
	b2Body* internalBody = createInternalBody(bodyDef);

	b2Vec2* internalChain = new b2Vec2[count];
	for (int index = 0; index < count; index++)
	{
		internalChain[index].x = PIXELS_TO_METERS(chain[index].mX);
		internalChain[index].y = PIXELS_TO_METERS(chain[index].mY);
	}

	// Attaching chain shape
	b2ChainShape chainShape;
	if (loop)
		chainShape.CreateLoop(internalChain, count);
	else
		chainShape.CreateChain(internalChain, count);
	delete [] internalChain;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &chainShape;
	fixtureDef.userData = bodyDef.mUserData;
	fixtureDef.friction = bodyDef.mFriction;
	fixtureDef.restitution = bodyDef.mRestitution;
	fixtureDef.isSensor = bodyDef.mIsSensor;
	fixtureDef.filter.categoryBits = bodyDef.mCategoryBits;
	fixtureDef.filter.maskBits = bodyDef.mFilterBits;
	internalBody->CreateFixture(&fixtureDef);

	if (handler) handler->mPosition = bodyDef.mPosition;

	TBodyHolder* holder = new TBodyHolder(internalBody, handler);
	body = mBodies.add(holder);

	if ((bodyDef.mType == EDynamicBody) && bodyDef.mIsActive)
	{
		holder->mDynamicId = mDynamicBodies.add(holder);
	}

	return E_SUCCESS;
}

void CBox2DPhysicsLayer::setActive(BODY_REF body, bool active)
{
	TBodyHolder* holder = mBodies.get(body);
	holder->mBody->SetActive(active);

	if (holder->mBody->GetType() == b2_dynamicBody)
	{
		if (active && (holder->mDynamicId == INVALID_ID))
		{
			holder->mDynamicId = mDynamicBodies.add(holder);
		}
		else if (!active && (holder->mDynamicId != INVALID_ID))
		{
			mDynamicBodies.remove(holder->mDynamicId);
			holder->mDynamicId = INVALID_ID;
		}
	}
}

void CBox2DPhysicsLayer::setVelocity(BODY_REF body, CVector2D& velocity)
{
	b2Vec2 internalVelocity(PIXELS_TO_METERS(velocity.mX) * mInverseTimeStep, PIXELS_TO_METERS(velocity.mY) * mInverseTimeStep);
	mBodies.get(body)->mBody->SetLinearVelocity(internalVelocity);
}

void CBox2DPhysicsLayer::setImpulse(BODY_REF body, CVector2D& point, CVector2D& impulse)
{
	b2Vec2 internalPoint(PIXELS_TO_METERS(point.mX), PIXELS_TO_METERS(point.mY));
	b2Vec2 internalImpulse(impulse.mX, impulse.mY);
	mBodies.get(body)->mBody->ApplyLinearImpulse(internalImpulse, internalPoint);
}

void CBox2DPhysicsLayer::setTransform(BODY_REF body, CVector2D& position, float angle)
{
	b2Vec2 internalPosition(PIXELS_TO_METERS(position.mX), PIXELS_TO_METERS(position.mY));
	TBodyHolder* holder = mBodies.get(body);
	holder->mBody->SetTransform(internalPosition, DEGREES_TO_RADIANS(angle));
	if (holder->mHandler) holder->mHandler->mPosition = position;
}

void CBox2DPhysicsLayer::getPosition(BODY_REF body, CVector2D& position)
{
	const b2Vec2& internalPosition = mBodies.get(body)->mBody->GetPosition();
	position.mX = METERS_TO_PIXELS(internalPosition.x);
	position.mY = METERS_TO_PIXELS(internalPosition.y);
}

float CBox2DPhysicsLayer::getAngle(BODY_REF body)
{
	return xmath_wrap_angle(RADIANS_TO_DEGREES(mBodies.get(body)->mBody->GetAngle()));
}

void CBox2DPhysicsLayer::getDistance(BODY_REF body, CVector2D& point, TBodyDistance& distance)
{
	mSimplexCache.count = 0;

	mEpsilonShape.m_p.x = PIXELS_TO_METERS(point.mX);
	mEpsilonShape.m_p.y = PIXELS_TO_METERS(point.mY);

	b2Body* internalBody = mBodies.get(body)->mBody;
	mDistanceInput.transformB.Set(internalBody->GetPosition(), internalBody->GetAngle());
	mDistanceInput.proxyB.Set(internalBody->GetFixtureList()->GetShape(), 0);
	mDistanceInput.useRadii = (internalBody->GetFixtureList()->GetShape()->GetType() == b2Shape::e_circle);

	b2Distance(&mDistanceOutput, &mSimplexCache, &mDistanceInput);

	distance.mClosestPoint.mX = METERS_TO_PIXELS(mDistanceOutput.pointB.x);
	distance.mClosestPoint.mY = METERS_TO_PIXELS(mDistanceOutput.pointB.y);
	distance.mDistance = METERS_TO_PIXELS(mDistanceOutput.distance);
}

bool CBox2DPhysicsLayer::rayCast(CVector2D& point1, CVector2D& point2, unsigned int filterBits, TRayCastResults& results)
{
	b2Vec2 internalPoint1(PIXELS_TO_METERS(point1.mX), PIXELS_TO_METERS(point1.mY));
	b2Vec2 internalPoint2(PIXELS_TO_METERS(point2.mX), PIXELS_TO_METERS(point2.mY));

	mReportedFixture = 0;
	mFilterBits = filterBits;
	mWorld->RayCast(this, internalPoint1, internalPoint2);

	if (mReportedFixture)
	{
		results.mUserData = mReportedFixture->GetUserData();

		results.mIntersection.mX = METERS_TO_PIXELS(mInitialIntersection.x);
		results.mIntersection.mY = METERS_TO_PIXELS(mInitialIntersection.y);
	}

	return (mReportedFixture != 0);
}

void CBox2DPhysicsLayer::searchBodies(CVector2D& position, float radius, unsigned int filterBits)
{
	b2AABB aabb;

	mSearchCenter.x = PIXELS_TO_METERS(position.mX);
	mSearchCenter.y = PIXELS_TO_METERS(position.mY);
	mSearchRadius = PIXELS_TO_METERS(radius);
	mFilterBits = filterBits;

	aabb.lowerBound.x = mSearchCenter.x - mSearchRadius;
	aabb.lowerBound.y = mSearchCenter.y - mSearchRadius;
	aabb.upperBound.x = mSearchCenter.x + mSearchRadius;
	aabb.upperBound.y = mSearchCenter.y + mSearchRadius;

	mBodySearchResults->mBodyCount = 0;

	mWorld->QueryAABB(this, aabb);
}

TBodySearchResults* CBox2DPhysicsLayer::getBodySearchResults()
{
	return mBodySearchResults;
}

void CBox2DPhysicsLayer::destroyBody(BODY_REF body)
{
	destroyBodyHolder(mBodies.get(body));
	mBodies.remove(body);
}

void CBox2DPhysicsLayer::setContactListener(IContactListener& listener)
{
	mContactListener = listener;
}

void CBox2DPhysicsLayer::simulate()
{
	mWorld->Step(mTimeStep, VELOCITY_ITERATIONS, POSITION_ITERATIONS);

	// Update body handlers
	TBodyHolder* holder;
	mDynamicBodies.reset();
	while ((holder = mDynamicBodies.next()) != 0)
	{
		holder->mHandler->mPosition.mX = METERS_TO_PIXELS(holder->mBody->GetPosition().x);
		holder->mHandler->mPosition.mY = METERS_TO_PIXELS(holder->mBody->GetPosition().y);
	}
}

bool CBox2DPhysicsLayer::isSimulating()
{
	return mWorld->IsLocked();
}

void CBox2DPhysicsLayer::destroyAllBodies()
{
	TBodyHolder* holder;
	mBodies.reset();

	while ((holder = mBodies.next()) != 0) destroyBodyHolder(holder);

	mBodies.clear();
}

int CBox2DPhysicsLayer::release()
{
	mRefCount--;

	if (mRefCount == 0)
	{
		delete mInstance;
		mInstance = 0;
	}

	return E_SUCCESS;
}

void CBox2DPhysicsLayer::BeginContact(b2Contact* contact)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();

	if (fixtureA->IsSensor() && !fixtureB->IsSensor())
	{
		mContactListener.onSensorTouched(fixtureA->GetUserData(), fixtureB->GetUserData());
	}
	else if (!fixtureA->IsSensor() && fixtureB->IsSensor())
	{
		mContactListener.onSensorTouched(fixtureB->GetUserData(), fixtureA->GetUserData());
	}
}

void CBox2DPhysicsLayer::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
	b2Fixture* fixtureA = contact->GetFixtureA();
	b2Fixture* fixtureB = contact->GetFixtureB();
	unsigned int solution = mContactListener.onContact(fixtureA->GetUserData(), fixtureB->GetUserData());

	if (solution & CONTACT_SOLUTION_IGNORE)
	{
		contact->SetEnabled(false);
	}

	if (solution & CONTACT_SOLUTION_STOP_BODY_A)
	{
		fixtureA->GetBody()->SetLinearVelocity(mZeroVelocity);
	}

	if (solution & CONTACT_SOLUTION_STOP_BODY_B)
	{
		fixtureB->GetBody()->SetLinearVelocity(mZeroVelocity);
	}
}

float32 CBox2DPhysicsLayer::ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction)
{
	if (fixture->IsSensor() || ((mFilterBits & fixture->GetFilterData().categoryBits) == 0))
	    return (-1); // ignore sensors and fixtures not matching filter

	mReportedFixture = fixture;

	mInitialIntersection.x = point.x;
	mInitialIntersection.y = point.y;

	return fraction;
}

bool CBox2DPhysicsLayer::ReportFixture(b2Fixture* fixture)
{
	if (fixture->IsSensor() || (fixture->GetUserData() == 0) || ((mFilterBits & fixture->GetFilterData().categoryBits) == 0))
	    return true; // ignore sensors and fixtures not matching filter or without user data

	const b2Vec2& sub = fixture->GetBody()->GetPosition() - mSearchCenter;
	const float radius = ((TBodyUserData*)(fixture->GetBody()->GetUserData()))->mFrameRadius + mSearchRadius;

	if (sub.LengthSquared() < (radius * radius))
	{
		mBodySearchResults->mUserData[mBodySearchResults->mBodyCount] = fixture->GetUserData();
		mBodySearchResults->mBodyCount++;
	}

	return true;
}

b2Body* CBox2DPhysicsLayer::createInternalBody(TBodyDef& bodyDef)
{
	b2BodyDef internalBodyDef;
	if (bodyDef.mType == EDynamicBody) internalBodyDef.type = b2_dynamicBody;
	internalBodyDef.position.x = PIXELS_TO_METERS(bodyDef.mPosition.mX);
	internalBodyDef.position.y = PIXELS_TO_METERS(bodyDef.mPosition.mY);
	internalBodyDef.angle = DEGREES_TO_RADIANS(bodyDef.mAngle);
	internalBodyDef.linearDamping = bodyDef.mDeceleration;
	internalBodyDef.angularDamping = bodyDef.mAngularDeceleration;
	internalBodyDef.fixedRotation = bodyDef.mIsFixedRotation;
	internalBodyDef.bullet = bodyDef.mIsBullet;
	internalBodyDef.active = bodyDef.mIsActive;

	// body user data
	TBodyUserData* bodyUserData = new TBodyUserData;
	bodyUserData->mFrameRadius = PIXELS_TO_METERS(bodyDef.mFrameRadius);
	internalBodyDef.userData = bodyUserData;

	if (mBodySearchResultsCapacity == mBodies.size())
	{
		mBodySearchResultsCapacity += (mBodySearchResultsCapacity >> 1) + 1;
		delete [] (mBodySearchResults->mUserData);
		mBodySearchResults->mUserData = new void*[mBodySearchResultsCapacity];
	}

	return mWorld->CreateBody(&internalBodyDef);
}

void CBox2DPhysicsLayer::destroyBodyHolder(TBodyHolder* holder)
{
	mWorld->DestroyBody(holder->mBody);

	if (holder->mDynamicId != INVALID_ID)
	{
		mDynamicBodies.remove(holder->mDynamicId);
	}

	delete holder;
}

// ==========================================================

IPhysicsLayer* obtainBox2DPhysicsLayer()
{
	return CBox2DPhysicsLayer::obtainInstance();
}