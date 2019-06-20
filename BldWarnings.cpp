#include "General.h"
#include "BldWarnings.h"
#include "engine.h"
#include "da.h"
#include "da_event.h"
#include "da_translation.h"
#include <string>

BldWarnings::BldWarnings()
{
	Console_Output("Loading Building Warnings Plugin... - By MasterCan\n");
	static BldWarnings Instance;
	Instance.Register_Object_Event(DAObjectEvent::DAMAGERECEIVED, DAObjectEvent::ObjectType::BUILDING, 10);
	Announce75 = true;
	Announce50 = true;
	Announce25 = true;
}
BldWarnings::~BldWarnings()
{
	Console_Output("Unloading Building Warnings... - By MasterCan\n");
	static BldWarnings Instance;
	Instance.Unregister_Object_Event(DAObjectEvent::DAMAGERECEIVED);
}

void BldWarnings::Damage_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type)
{
	if (Announce75 && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) > ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.74f) && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) < ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.76f))
	{
		DA::Team_Color_Message_With_Team_Color(Victim->Get_Player_Type(), "%sThe %s health at 75 percent", DA::Get_Message_Prefix(), DATranslationManager::Translate(Victim));
		if (Victim->Get_Player_Type() == 0) { DA::Create_2D_Sound_Team(0, "M00EVAN_DSGN0070I1EVAN_SND.wav"); }
		if (Victim->Get_Player_Type() == 1) { DA::Create_2D_Sound_Team(1, "M00EVAG_DSGN0066I1EVAG_SND.wav"); }
		Announce75 = false;
		Start_Timer(1, 10);
	}
	else if (Announce50 && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) > ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.49f) && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) < ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.51f))
	{
		DA::Team_Color_Message_With_Team_Color(Victim->Get_Player_Type(), "%sThe %s health at 50 percent", DA::Get_Message_Prefix(), DATranslationManager::Translate(Victim));
		if (Victim->Get_Player_Type() == 0) { DA::Create_2D_Sound_Team(0, "M00EVAN_DSGN0071I1EVAN_SND.wav"); }
		if (Victim->Get_Player_Type() == 1) { DA::Create_2D_Sound_Team(1, "M00EVAG_DSGN0067I1EVAG_SND.wav"); }
		Announce50 = false;
		Start_Timer(2, 10);
	}
	else if (Announce25 && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) > ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.24f) && (Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim)) < ((Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim)) * 0.26f))
	{
		DA::Team_Color_Message_With_Team_Color(Victim->Get_Player_Type(), "%sThe %s health at 25 percent", DA::Get_Message_Prefix(), DATranslationManager::Translate(Victim));
		if (Victim->Get_Player_Type() == 0) { DA::Create_2D_Sound_Team(0, "M00EVAN_DSGN0072I1EVAN_SND.wav"); }
		if (Victim->Get_Player_Type() == 1) { DA::Create_2D_Sound_Team(1, "M00EVAG_DSGN0068I1EVAG_SND.wav"); }
		Announce25 = false;
		Start_Timer(3, 10);
	}
}

void BldWarnings::Timer_Expired(int Number, unsigned int Data)
{
	if (Number == 1)
	{
		Announce75 = true;
	}
	else if (Number == 2)
	{
		Announce50 = true;
	}
	else if (Number == 3)
	{
		Announce25 = true;
	}
}

BldWarnings thisplugin;

extern "C" __declspec(dllexport) DAEventClass* Plugin_Init()
{
	return &thisplugin;
}