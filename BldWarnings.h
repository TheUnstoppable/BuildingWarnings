#ifndef BLDWARN
#define BLDWARN
#pragma once

#include "da.h"
#include "da_chatcommand.h"
#include "da_gameobj.h"
#include "da_ssgm.h"
#include "da_event.h"

class BldWarnings : public DAEventClass
{
public:
	BldWarnings();
	~BldWarnings();
	bool Announce75;
	bool Announce50;
	bool Announce25;
	virtual void Damage_Event(DamageableGameObj *Victim, ArmedGameObj *Damager, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type);
	virtual void Timer_Expired(int Number, unsigned int Data);
};

#endif