#include "..\content\content_refs.h"
#include "..\content\content_provider.h"
#include "..\content\texture_content.h"

#include "player_entity.h"

#define PLAYER_SPEED 1.5f
#define PLAYER_MASS 80.0f
#define PLAYER_RADIUS 8.5f

#define PLAYER_STATE_ALIVE 0
#define PLAYER_STATE_PREDEAD 1
#define PLAYER_STATE_DEAD 2

CPlayerEntity::CPlayerEntity():
IEntity(ENTITY_GROUP_HUMAN),
mAngle(0.0f),
mHealth(100),
mState(PLAYER_STATE_ALIVE)
{
	mModel = new CHumanModel((CTextureContent*)CContentProvider::instance()->querySecondaryContent(ETextureContentHuman01), 23, 1);
	mSight = new CCursorGUIElement();

	mWeapon = new CDefenderHandgun();
	CSpriteManager::instance()->addSprite(mModel, mModel->getDefaultDepth());

	CGUIManager::instance()->addGUIElement(mSight);
}

CPlayerEntity::~CPlayerEntity()
{
	CGUIManager::instance()->removeGUIElement(mSight);

	CSpriteManager::instance()->removeSprite(mModel);

	delete mSight;
	delete mWeapon;
	delete mModel;
}

CVector2D& CPlayerEntity::action()
{
	IInputLayer* input = CLayerManager::instance()->getInputLayer();
	CWorld* world = CWorld::instance();
	CVector2D playerPosition = getPosition();

	if (mState == PLAYER_STATE_PREDEAD)
	{
		world->setActive(this, false);
		mState = PLAYER_STATE_DEAD;
		mVelocity.mX = mVelocity.mY = 0.0f;
		return mVelocity;
	}

	float direction = -1.0f;
	int action = EHumanActionHandgunStay;

	if (input->keyPressed(TKEY_W))
	{
		direction = mAngle;
		action = EHumanActionHandgunWalk;
	}
	else if (input->keyPressed(TKEY_S))
	{
		direction = XMATH_WRAP_UPPER(mAngle + 180);
		action = EHumanActionHandgunWalk;
	}
	else if (input->keyPressed(TKEY_A))
	{
		direction = XMATH_WRAP_UPPER(mAngle + 90);
		action = EHumanActionHandgunLeftStrafe;
	}
	else if (input->keyPressed(TKEY_D))
	{
		direction = XMATH_WRAP_LOWER(mAngle - 90);
		action = EHumanActionHandgunRightStrafe;
	}

	if (world->getMouseController()->isButtonPressed(LEFT_MOUSE_BUTTON))
	{
		mWeapon->fire();
	}

	if (direction >= 0)
	{
		mVelocity = CVector2D::getUnitVector(direction) * PLAYER_SPEED;
	}
	else
	{
		mVelocity.mX = mVelocity.mY = 0.0f;
	}
	mModel->setAction(action);

	world->getViewPort()->setPosition(playerPosition);

	mModel->setPosition(playerPosition);
	mWeapon->setPosition(playerPosition);
	CVector2D sub = mSight->getPosition() - mModel->getPosition();
	mAngle = XMATH_WRAP_LOWER(RADIANS_TO_DEGREES(-atan2(sub.mY, sub.mX)));
	mModel->setAngle(mAngle);
	mWeapon->setAngle(mAngle);

	return mVelocity;
}

void CPlayerEntity::applyDamage(CVector2D* position, TDamageType damageType, int damageValue)
{
	if (mState == PLAYER_STATE_ALIVE)
	{
		mHealth -= damageValue;
		if (mHealth <= 0)
		{
			mState = PLAYER_STATE_PREDEAD;
			mModel->setAction(EHumanActionDeath);
		}
	}
}

bool CPlayerEntity::onContact(IEntity* entity)
{
	return false;
}

float CPlayerEntity::getAngle()
{
	return mAngle;
}

float CPlayerEntity::getRadius()
{
	return PLAYER_RADIUS;
}

float CPlayerEntity::getMass()
{
	return PLAYER_MASS;
}

void CPlayerEntity::onDeregistered()
{
	delete this;
}

void CPlayerEntity::registerToWorld(CVector2D& position)
{
	TCircleShape shape(PLAYER_RADIUS);
	TEntityDef def(shape);
	def.mPosition = position;
	def.mFrameRadius = PLAYER_RADIUS;

	mModel->setPosition(position);

	CWorld::instance()->registerEntity(this, def);
}