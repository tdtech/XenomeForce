#pragma once

#include "..\gui\cursor_gui_element.h"
#include "..\sprite\human_model.h"
#include "..\weapon\defender_handgun.h"
#include "world.h"

class CPlayerEntity: public IEntity
{
public:
	CPlayerEntity();
	~CPlayerEntity();

	CVector2D& action();
	void applyDamage(CVector2D* position, TDamageType damageType, int damageValue);
	bool onContact(IEntity* entity);

	float getAngle();
	float getRadius();

	float getMass();

	void onDeregistered();

	void registerToWorld(CVector2D& position);

private:
	CHumanModel*       mModel;
	CDefenderHandgun*  mWeapon;
	CCursorGUIElement* mSight;

	float              mAngle;
	int                mHealth;
	int                mState;

	CVector2D          mVelocity;
};
