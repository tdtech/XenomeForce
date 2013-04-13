#pragma once

#include "..\common\common.h"
#include "..\common\xmath.h"
#include "..\common\collection.h"

#include "..\layer\layer_manager.h"

#include "..\content\content_builder.h"
#include "..\controller\fps_controller.h"
#include "..\controller\mouse_controller.h"
#include "..\gui\gui_manager.h"
#include "..\light\light_manager.h"
#include "..\sprite\sprite_manager.h"
#include "..\landscape\landscape_builder.h"
#include "..\viewport\viewport.h"
#include "world_time.h"

#define MAX_MARKED_ENTITIES_PER_FRAME 128
#define MAX_MARKED_SENSORS_PER_FRAME 128
#define MAX_EXPLOSIONS_PER_FRAME 64

// ==== SHAPE ====

class IShape
{
	friend class CWorld;

public:
	virtual ~IShape() {}

protected:
	virtual BODY_REF buildShape(TBodyDef& bodyDef, TBodyHandler* handler) = 0;
};

// Rectangle shape
class TRectangleShape: public IShape
{
public:
	TRectangleShape(float width, float height);
	~TRectangleShape() {}

protected:
	BODY_REF buildShape(TBodyDef& bodyDef, TBodyHandler* handler);

private:
	float mWidth;
	float mHeight;
};

// Circle shape
class TCircleShape: public IShape
{
public:
	TCircleShape(float radius);
	~TCircleShape() {}

protected:
	BODY_REF buildShape(TBodyDef& bodyDef, TBodyHandler* handler);

private:
	float mRadius;
};

// ==== ENTITY ====

// Entity groups
#define ENTITY_GROUP_HUMAN     0x0001
#define ENTITY_GROUP_MECHANISM 0x0002
#define ENTITY_GROUP_CREATURE  0x0004
#define ENTITY_GROUP_BULLET    0x0008
#define ENTITY_GROUP_DEBRIS    0x0010

// Damage types
enum TDamageType
{
	 EDamageCommon
	,EDamageFire
	,EDamageIce
	,EDamageImmobilization
	// add more damage types...
};

// Entity definition
struct TEntityDef
{
	TEntityDef(IShape& shape):
	mShape(shape)
	{
		mAngle = 0.0f;
		mFrameRadius = 0.0f;
		mRestitution = 0.3f;
		mGroupFilter = 0xFFFF;
	}

	CVector2D    mPosition;
	float        mAngle;
	float        mFrameRadius; // for searching entities
	IShape&      mShape;
	float        mRestitution;
	unsigned int mGroupFilter;
};

// Entity interface
class IEntity
{
	friend class CWorld;

public:
	IEntity(unsigned int group);
	virtual ~IEntity() {}

	virtual CVector2D& action() = 0;
	virtual void applyDamage(CVector2D* position, TDamageType damageType, int damageValue) = 0;
	virtual bool onContact(IEntity* entity) = 0; // true if contact has been consumed

	void setTransform(CVector2D& pos, float angle);
	inline CVector2D& getPosition() { return mBodyHandler.mPosition; }
	inline unsigned int getGroup() { return mGroup; }

	virtual float getAngle();
	virtual float getRadius() = 0;
	virtual float getMass() = 0;

	/* Called when actual deregistration of entity occures.
	   After this entity does not belong to world and can be destroyed. */
	virtual void onDeregistered() = 0;

private:
	BODY_REF     mBodyRef;
	TBodyHandler mBodyHandler;
	int          mEntityId;
	bool         mActive;
	unsigned int mGroup;
};

// ==== EVENT ====

class IEvent
{
	friend class CWorld;

public:
	virtual ~IEvent() {}

	virtual void process() = 0;

private:
	int mEventId;
};

// ==== SENSOR ====
// IMPORTANT: sensors are not destroyed by world

class ISensor
{
	friend class CWorld;

public:
	virtual ~ISensor() {}

	virtual void onTouch(IEntity* entity) = 0;

	/* Called when actual deregistration of sensor occures.
	   After this sensor does not belong to world and can be destroyed. */
	virtual void onDeregistered() = 0;

private:
	BODY_REF mBodyRef;
	int      mSensorId;
};

// ==== WORLD ====

struct TWorldRayCastResults
{
	IEntity*  mEntity;
	CVector2D mIntersection;
};

class CWorld: public IContactListener
{
public:
	static void init(CViewPort* viewPort);
	static void release();
	static inline CWorld* instance() { return mInstance; }

	void registerEntity(IEntity* entity, TEntityDef& entityDef); // newly registered entity is inactive by default
	void setActive(IEntity* entity, bool active); // active entity interacts with world
	void deregisterEntity(IEntity* entity);

	int searchEntities(CVector2D& pos, float radius, unsigned int groupFilter = 0xFFFF);
	IEntity** getSearchResults();
	void applyDamage(CVector2D& pos, float radius, TDamageType damageType, int damageValue, unsigned int groupFilter = 0xFFFF);
	void applyShake(CVector2D& pos, float radius);
	void applyExplosion(CVector2D& pos, float radius, TDamageType damageType, int damageValue, unsigned int groupFilter = 0xFFFF);
	bool rayCast(CVector2D& point1, CVector2D& point2, TWorldRayCastResults& results, unsigned int groupFilter = 0xFFFF);

	void registerEvent(IEvent* event);
	void deregisterEvent(IEvent* event);

	void registerSensor(ISensor* sensor, IShape& shape, CVector2D& pos);
	void deregisterSensor(ISensor* sensor);

	CWorldTime& getWorldTime();
	CViewPort* getViewPort();
	CMouseController* getMouseController();

	void simulate();
	void clear();

	// IContactListener implementation
	unsigned int onContact(void* userDataA, void* userDataB);
	void onSensorTouched(void* sensorUserData, void* bodyUserData);

private:
	struct TExplosionSpot
	{
		CVector2D mPosition;
		float     mRadius;
	};

	CWorld(CViewPort* viewPort);
	~CWorld();

	void actionEntities();
	void processEvents();

	void triggerExplosions();

	void deregisterAllEntities();
	void deregisterAllEvents();
	void deregisterAllSensors();

	void deregisterMarkedEntities();
	void deregisterMarkedSensors();

	void deregisterEntityInternal(IEntity* entity);
	void deregisterSensorInternal(ISensor* sensor);

private:
	CCollection<IEntity>  mEntities;
	CCollection<IEvent>   mEvents;
	CCollection<ISensor>  mSensors;

	IEntity*              mMarkedEntities[MAX_MARKED_ENTITIES_PER_FRAME];
	int                   mMarkedEntitiesCount;
	ISensor*              mMarkedSensors[MAX_MARKED_SENSORS_PER_FRAME];
	int                   mMarkedSensorsCount;

	TExplosionSpot        mExplosions[MAX_EXPLOSIONS_PER_FRAME];
	int                   mExplosionsCount;

	TBodyDistance         mBodyDistance;
	CVector2D             mImpulse;

	CWorldTime            mWorldTime;

	CViewPort*            mViewPort;

	CFPSController        mFPSController;
	CMouseController*     mMouseController;

	IEntity**             mSearchResults;
	TRayCastResults       mRayCastResults;

	static CWorld*        mInstance;
};