/*
	Building Warnings
	Copyright 2020 Unstoppable

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef BLDWARN_INCLUDES
#define BLDWARN_INCLUDES

#include "da_gamefeature.h"
#include "da_event.h"

#define CompileTime StringFormat("%s %s", __DATE__, __TIME__)
#define Version 1.3f

enum WarnState
{
	WARNSTATE_REPAIRING = 0,
	WARNSTATE_75,
	WARNSTATE_50,
	WARNSTATE_25
};

struct FormatParams
{
	WarnState State;
	StringClass Message;
	int Team;
	GameObject* Building;
	GameObject* Damager;
	StringClass *Out;
};

class UP_Building_Warnings : public DAEventClass, public DAGameFeatureClass {
public:
	void SendStateMessage(FormatParams *Params);
	void FormatStateMessage(FormatParams *Params, StringClass& Out);
protected:
	void Init() override;
	void Settings_Loaded_Event() override;
	void Damage_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type) override;
	void Timer_Expired(int Number, unsigned int Data) override;
	~UP_Building_Warnings();
private:
	static const char Sounds[4][2][50];
	static DASettingsClass *INI;

	static StringClass Format75;
	static StringClass Format50;
	static StringClass Format25;
	static StringClass FormatRepairing;
	static DynamicVectorClass<StringClass> IncludeList;
	static DynamicVectorClass<StringClass> ExcludeList;
	static bool ReportVehicleBuildings;
	static bool ShowEnemy;
	static bool UseDAPrefix;
	static bool UseTeamColors;
	static float PercentCooldown;
	static float RepairCooldown;
	static Vector3 Color75;
	static Vector3 Color50;
	static Vector3 Color25;
	static Vector3 ColorRepairing;
	static bool Enable75;
	static bool Enable50;
	static bool Enable25;
	static bool EnableRepairing;
};

#endif