#include "world.h"

#define ENTITIES_INITIAL_CAPACITY 512
#define EVENTS_INITIAL_CAPACITY 128
#define SENSORS_INITIAL_CAPACITY 64

#define DEFAULT_FRICTION 0.3f
#define DEFAULT_DECELERATION 2.5f
#define DEFAULT_ANGULAR_DECELERATION 3.0f

#define IMPULSE_MULTIPLIER 0.3f // TODO: find appropriate value
#define SHAKE_MULTIPLIER 0.5f // TODO: find appropriate value
#define MAX_SHAKE_DISTANCE 147456.0f // TODO: find appropriate value
#define EPSILON_DISTANCE 0.1f

TRectangleShape::TRectangleShape(float width, float height):
mWidth(width),
mHeight(height)
{}

BODY_REF TRectangleShape::buildShape(TBodyDef& bodyDef, TBodyHandler* handler)
{
	BODY_REF ref;
	CLayerManager::instance()->getPhysicsLayer()->createRectangleBody(bodyDef, mWidth, mHeight, handler, ref);
	return ref;
}

TCircleShape::TCircleShape(float radius):
mRadius(radius)
{}

BODY_REF TCircleShape::buildShape(TBodyDef& bodyDef, TBodyHandler* handler)
{
	BODY_REF ref;
	CLayerManager::instance()->getPhysicsLayer()->createCircleBody(bodyDef, mRadius, handler, ref);
	return ref;
}

//-----------------------------------------------------

IEntity::IEntity(unsigned int group):
mGroup(group)
{}

void IEntity::setTransform(CVector2D& pos, float angle)
{
	CLayerManager::instance()->getPhysicsLayer()->setTransform(mBodyRef, pos, angle);
}

float IEntity::getAngle()
{
	return CLayerManager::instance()->getPhysicsLayer()->getAngle(mBodyRef);
}

//-----------------------------------------------------

CWorld* CWorld::mInstance = 0;

void CWorld::init(CViewPort* viewPort)
{
	release();
	mInstance = new CWorld(viewPort);
}

void CWorld::release()
{
	delete mInstance;
	mInstance = 0;
}

CWorld::CWorld(CViewPort* viewPort):
mEntities(ENTITIES_INITIAL_CAPACITY),
mEvents(EVENTS_INITIAL_CAPACITY),
mSensors(SENSORS_INITIAL_CAPACITY),
mViewPort(viewPort),
mMarkedEntitiesCount(0),
mMarkedSensorsCount(0),
mExplosionsCount(0)
{
	TRect clipRect;
	clipRect.mX0 = clipRect.mY0 = 0;
	clipRect.mX1 = mViewPort->getWidth();
	clipRect.mY1 = mViewPort->getHeight();
	mMouseController = new CMouseController(clipRect);
}

CWorld::~CWorld()
{
	clear();

	delete mMouseController;
	delete mViewPort;
}

void CWorld::registerEntity(IEntity* entity, TEntityDef& entityDef)
{
	unsigned int group = entity->mGroup;

	TBodyDef bodyDef;
	bodyDef.mPosition = entityDef.mPosition;
	bodyDef.mAngle = entityDef.mAngle;
	bodyDef.mFrameRadius = entityDef.mFrameRadius;
	bodyDef.mType = EDynamicBody;
	bodyDef.mIsActive = false;
	bodyDef.mIsBullet = (group & ENTITY_GROUP_BULLET) > 0;
	bodyDef.mIsFixedRotation = !(group & ENTITY_GROUP_DEBRIS);
	bodyDef.mMass = entity->getMass();
	bodyDef.mFriction = DEFAULT_FRICTION;
	bodyDef.mRestitution = entityDef.mRestitution;
	bodyDef.mDeceleration = DEFAULT_DECELERATION;
	bodyDef.mAngularDeceleration = DEFAULT_ANGULAR_DECELERATION;
	bodyDef.mCategoryBits = group;
	bodyDef.mFilterBits = entityDef.mGroupFilter;
	bodyDef.mUserData = entity;

	entity->mBodyRef = entityDef.mShape.buildShape(bodyDef, &entity->mBodyHandler);

	entity->mEntityId = mEntities.add(entity);
	entity->mActive = false;
}

void CWorld::setActive(IEntity* entity, bool active)
{
	if (entity->mActive != active) {
		CLayerManager::instance()->getPhysicsLayer()->setActive(entity->mBodyRef, active);
		entity->mActive = active;
	}
}

void CWorld::deregisterEntity(IEntity* entity)
{
	mMarkedEntities[mMarkedEntitiesCount++] = entity;
}

int CWorld::searchEntities(CVector2D& pos, float radius, unsigned int groupFilter)
{
	IPhysicsLayer* physicsLayer = CLayerManager::instance()->getPhysicsLayer();

	physicsLayer->searchBodies(pos, radius, groupFilter);
	TBodySearchResults* results = physicsLayer->getBodySearchResults();
	mSearchResults = (IEntity**)results->mUserData;

	return results->mBodyCount;
}

IEntity** CWorld::getSearchResults()
{
	return mSearchResults;
}

void CWorld::applyDamage(CVector2D& pos, float radius, TDamageType damageType, int damageValue, unsigned int groupFilter)
{
	int count = searchEntities(pos, radius, groupFilter);
	for (int index = 0; index < count; index++)
	{
		mSearchResults[index]->applyDamage(0, damageType, damageValue);
	}
}

void CWorld::applyShake(CVector2D& pos, float radius)
{
	CVector2D sub = mViewPort->getPosition() - pos;
	float extendedMaxShakeDistance = MAX_SHAKE_DISTANCE + radius * radius;
	float distanceToViewPort = sub * sub; // squared
	if (distanceToViewPort < extendedMaxShakeDistance)
	{
		mViewPort->applyShake(((extendedMaxShakeDistance - distanceToViewPort) / extendedMaxShakeDistance) *
		                      radius * SHAKE_MULTIPLIER);
	}
}

void CWorld::applyExplosion(CVector2D& pos, float radius, TDamageType damageType, int damageValue, unsigned int groupFilter)
{
	applyDamage(pos, radius, damageType, damageValue, groupFilter);

	// do actual explosion at the next world tick to apply them to debris
	if (mExplosionsCount < MAX_EXPLOSIONS_PER_FRAME)
	{
		mExplosions[mExplosionsCount].mPosition = pos;
		mExplosions[mExplosionsCount++].mRadius = radius;
	}
}

bool CWorld::rayCast(CVector2D& point1, CVector2D& point2, TWorldRayCastResults& results, unsigned int groupFilter)
{
	if (CLayerManager::instance()->getPhysicsLayer()->rayCast(point1, point2, groupFilter, mRayCastResults))
	{
		results.mEntity = (IEntity*)mRayCastResults.mUserData;
		results.mIntersection = mRayCastResults.mIntersection;

		return true;
	}

	return false;
}

void CWorld::registerEvent(IEvent* event)
{
	event->mEventId = mEvents.add(event);
}

void CWorld::deregisterEvent(IEvent* event)
{
	mEvents.remove(event->mEventId);
}

void CWorld::registerSensor(ISensor* sensor, IShape& shape, CVector2D& pos)
{
	TBodyDef bodyDef;
	bodyDef.mPosition = pos;
	bodyDef.mIsSensor = true;
	bodyDef.mUserData = sensor;

	sensor->mBodyRef = shape.buildShape(bodyDef, 0);
	sensor->mSensorId = mSensors.add(sensor);
}

void CWorld::deregisterSensor(ISensor* sensor)
{
	mMarkedSensors[mMarkedSensorsCount++] = sensor;
}

CWorldTime& CWorld::getWorldTime()
{
	return mWorldTime;
}

CViewPort* CWorld::getViewPort()
{
	return mViewPort;
}

CMouseController* CWorld::getMouseController()
{
	return mMouseController;
}

void CWorld::simulate()
{
	mFPSController.beginFrame();
	if (CLayerManager::instance()->getRenderLayer()->beginScene())
	{
		// Drawing phase
		mViewPort->draw();

		CSpriteManager::instance()->drawDecals(mViewPort->getLandscape());
		CSpriteManager::instance()->drawSprites();
		CLightManager::instance()->drawLights();
		CGUIManager::instance()->drawGUIElements();

		CLayerManager::instance()->getRenderLayer()->endScene();

		// Logic phase
		CLayerManager::instance()->getInputLayer()->readKeyBoard();
		mMouseController->updateState();
		triggerExplosions();
		actionEntities();
		processEvents();
		CSpriteManager::instance()->animateSprites();
		CLightManager::instance()->animateLights();
		CGUIManager::instance()->handleEvents();
		deregisterMarkedSensors();
		deregisterMarkedEntities();
		CLayerManager::instance()->getPhysicsLayer()->simulate();
		mViewPort->update();
		mWorldTime.nextTime();

		// Presenting phase
		CLayerManager::instance()->getRenderLayer()->presentScene();
	}
	else
	{
		// TODO: stop music and all sounds
	}
	mFPSController.endFrame();
}

void CWorld::clear()
{
	mMarkedEntitiesCount = mMarkedSensorsCount = mExplosionsCount = 0;

	deregisterAllEntities();
	deregisterAllEvents();
	deregisterAllSensors();

	CLayerManager::instance()->getPhysicsLayer()->destroyAllBodies();
}

unsigned int CWorld::onContact(void* userDataA, void* userDataB)
{
	unsigned int solution = CONTACT_SOLUTION_CONTINUE;

	if ((userDataA) && (userDataB))
	{
		IEntity* entityA = (IEntity*)userDataA;
		IEntity* entityB = (IEntity*)userDataB;

		if (entityA->onContact(entityB))
		{
			solution |= CONTACT_SOLUTION_IGNORE | CONTACT_SOLUTION_STOP_BODY_A;
		}

		if (entityB->onContact(entityA))
		{
			solution |= CONTACT_SOLUTION_IGNORE | CONTACT_SOLUTION_STOP_BODY_B;
		}
	}

	return solution;
}

void CWorld::onSensorTouched(void* sensorUserData, void* bodyUserData)
{
	if (bodyUserData)
	{
		ISensor* sensor = (ISensor*)sensorUserData;
		sensor->onTouch((IEntity*)bodyUserData);
	}
}

void CWorld::actionEntities()
{
	IEntity* entity;
	mEntities.reset();
	while ((entity = mEntities.next()) != 0)
	{
		if (entity->mActive)
		{
			CVector2D& velocity = entity->action();

			if ((entity->mGroup & ENTITY_GROUP_DEBRIS) == 0)
				CLayerManager::instance()->getPhysicsLayer()->setVelocity(entity->mBodyRef, velocity);
		}
	}
}

void CWorld::processEvents()
{
	IEvent* event;
	mEvents.reset();
	while ((event = mEvents.next()) != 0) event->process();
}

void CWorld::triggerExplosions()
{
	for (int explosion = 0; explosion < mExplosionsCount; explosion++)
	{
		float explosionRadius = mExplosions[explosion].mRadius;

		int count = searchEntities(mExplosions[explosion].mPosition, explosionRadius);
		for (int index = 0; index < count; index++)
		{
			if (mSearchResults[index]->mGroup & ENTITY_GROUP_DEBRIS)
			{
				CLayerManager::instance()->getPhysicsLayer()->getDistance(mSearchResults[index]->mBodyRef,
				        mExplosions[explosion].mPosition, mBodyDistance);

				if (mBodyDistance.mDistance < EPSILON_DISTANCE)
				{
					mImpulse = CVector2D::getUnitVector(rand() % 360) * (explosionRadius * IMPULSE_MULTIPLIER);

					CLayerManager::instance()->getPhysicsLayer()->setImpulse(mSearchResults[index]->mBodyRef,
					        mExplosions[explosion].mPosition, mImpulse);
				}
				else if (mBodyDistance.mDistance < explosionRadius)
				{
					mImpulse = (mBodyDistance.mClosestPoint - mExplosions[explosion].mPosition) *
					         (((explosionRadius - mBodyDistance.mDistance) * IMPULSE_MULTIPLIER) / mBodyDistance.mDistance);

					CLayerManager::instance()->getPhysicsLayer()->setImpulse(mSearchResults[index]->mBodyRef,
					        mBodyDistance.mClosestPoint, mImpulse);
				}
			}
		}

		applyShake(mExplosions[explosion].mPosition, explosionRadius);
	}
	mExplosionsCount = 0;
}

void CWorld::deregisterAllEntities()
{
	IEntity* entity;
	mEntities.reset();
	while ((entity = mEntities.next()) != 0) deregisterEntityInternal(entity);
	mEntities.clear();
}

void CWorld::deregisterAllEvents()
{
	IEvent* event;
	mEvents.reset();
	while ((event = mEvents.next()) != 0) deregisterEvent(event);
	mEvents.clear();
}

void CWorld::deregisterAllSensors()
{
	ISensor* sensor;
	mSensors.reset();
	while ((sensor = mSensors.next()) != 0) deregisterSensorInternal(sensor);
	mSensors.clear();
}

void CWorld::deregisterMarkedEntities()
{
	for (int id = 0; id < mMarkedEntitiesCount; id++)
	{
		deregisterEntityInternal(mMarkedEntities[id]);
	}
	mMarkedEntitiesCount = 0;
}

void CWorld::deregisterMarkedSensors()
{
	for (int id = 0; id < mMarkedSensorsCount; id++)
	{
		deregisterSensorInternal(mMarkedSensors[id]);
	}
	mMarkedSensorsCount = 0;
}

void CWorld::deregisterEntityInternal(IEntity* entity)
{
	CLayerManager::instance()->getPhysicsLayer()->destroyBody(entity->mBodyRef);

	mEntities.remove(entity->mEntityId);

	entity->onDeregistered();
}

void CWorld::deregisterSensorInternal(ISensor* sensor)
{
	CLayerManager::instance()->getPhysicsLayer()->destroyBody(sensor->mBodyRef);
	mSensors.remove(sensor->mSensorId);

	sensor->onDeregistered();
}