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

#include "General.h"
//Engine
#include "engine_def.h"
#include "engine_io.h"
#include "engine_math.h"
#include "engine_obj.h"
#include "engine_player.h"
#include "engine_tdb.h"
//DA
#include "da.h"
#include "da_settings.h"
#include "da_translation.h"
//Building Warnings
#include "BldWarn.h"

#define Cooldown(WState, Data) Start_Timer(static_cast<int>(WState), WState == WarnState::WARNSTATE_REPAIRING ? RepairCooldown : PercentCooldown, false, Data)

const char UP_Building_Warnings::Sounds[4][2][50] = {
        {
            "m00evan_dsgn0070i1evan_snd.wav",
            "m00evag_dsgn0066i1evag_snd.wav"
        },
        {
            "m00evan_dsgn0071i1evan_snd.wav",
            "m00evag_dsgn0067i1evag_snd.wav"
        },
        {
            "m00evan_dsgn0072i1evan_snd.wav",
            "m00evag_dsgn0068i1evag_snd.wav"
        },
        {
            "m00evan_dsgn0015i1evan_snd.wav",
            "m00evag_dsgn0018i1evag_snd.wav"
        },
};

DASettingsClass* UP_Building_Warnings::INI = 0;

// Using static, because values are unavailable outside of Settings_Loaded_Event when non-static.
// I don't have time, at least works.
StringClass UP_Building_Warnings::Format75;
StringClass UP_Building_Warnings::Format50;
StringClass UP_Building_Warnings::Format25;
StringClass UP_Building_Warnings::FormatRepairing;
DynamicVectorClass<StringClass> UP_Building_Warnings::IncludeList;
DynamicVectorClass<StringClass> UP_Building_Warnings::ExcludeList;
bool UP_Building_Warnings::ReportVehicleBuildings;
bool UP_Building_Warnings::ShowEnemy;
bool UP_Building_Warnings::UseDAPrefix;
bool UP_Building_Warnings::UseTeamColors;
float UP_Building_Warnings::PercentCooldown;
float UP_Building_Warnings::RepairCooldown;
Vector3 UP_Building_Warnings::Color75;
Vector3 UP_Building_Warnings::Color50;
Vector3 UP_Building_Warnings::Color25;
Vector3 UP_Building_Warnings::ColorRepairing;

void UP_Building_Warnings::Init()
{
    //The default settings.

    Format75 = "The {TEAM} {NAME} health is at 75 percent.";
    Format50 = "The {TEAM} {NAME} health is at 50 percent.";
    Format25 = "The {TEAM} {NAME} health is at 25 percent.";
    FormatRepairing = "{PLAYER} is repairing the {TEAM} {NAME}.";

    Color75 = Vector3(255.0f, 255.0f, 255.0f);
    Color50 = Vector3(255.0f, 255.0f, 255.0f);
    Color25 = Vector3(255.0f, 255.0f, 255.0f);
    ColorRepairing = Vector3(255.0f, 255.0f, 255.0f);

    ReportVehicleBuildings = true;
    ShowEnemy = false;
    UseDAPrefix = true;
    UseTeamColors = true;

    PercentCooldown = 20.0f;
    RepairCooldown = 30.0f;

    IncludeList = DynamicVectorClass<StringClass>();
    ExcludeList = DynamicVectorClass<StringClass>();

    if (INI)
        delete INI;

    INI = new DASettingsClass("BuildingWarnings.ini");

    //Events

    Register_Event(DAEvent::SETTINGSLOADED);
    Register_Object_Event(DAObjectEvent::DAMAGERECEIVED, DAObjectEvent::ALL, INT_MAX);
    
    Start_Timer(100, 1.0f, true, 0);

    Console_Output("%sBuilding Warnings successfully initialized!\n", DA::Get_Message_Prefix());
    Console_Output("%sVersion %.2f built at %s\n", DA::Get_Message_Prefix(), Version, CompileTime);
}

void UP_Building_Warnings::Settings_Loaded_Event()
{
    INI->Reload();

    Format75 = INI->Get_String(Format75, "Messages", "Percent75", "The {TEAM} {NAME} health is at 75 percent.");;
    Format50 = INI->Get_String(Format50, "Messages", "Percent50", "The {TEAM} {NAME} health is at 50 percent.");
    Format25 = INI->Get_String(Format25, "Messages", "Percent25", "The {TEAM} {NAME} health is at 25 percent.");
    FormatRepairing = INI->Get_String(FormatRepairing, "Messages", "Repairing", "{PLAYER} is repairing the {TEAM} {NAME}.");

    PercentCooldown = static_cast<float>(INI->Get_Int("Settings", "PercentCooldown", 20));
    RepairCooldown = static_cast<float>(INI->Get_Int("Settings", "RepairCooldown", 30));

    INI->Get_Vector3(Color75, "Colors", "Percent75", Vector3(255.0f, 255.0f, 255.0f));
    INI->Get_Vector3(Color50, "Colors", "Percent50", Vector3(255.0f, 255.0f, 255.0f));
    INI->Get_Vector3(Color25, "Colors", "Percent25", Vector3(255.0f, 255.0f, 255.0f));
    INI->Get_Vector3(ColorRepairing, "Colors", "Repairing", Vector3(255.0f, 255.0f, 255.0f));

    ReportVehicleBuildings = INI->Get_Bool("Settings", "ReportVehicleBuildings", true);
    ShowEnemy = INI->Get_Bool("Settings", "ShowEnemy", false);
    UseDAPrefix = INI->Get_Bool("Settings", "UseDAPrefix", true);
    UseTeamColors = INI->Get_Bool("Settings", "UseTeamColors", true);

    IncludeList.Clear();
    INISection* IncludeSection = INI->Get_Section("Includes");
    for (INIEntry* Entry = IncludeSection->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next())
    {
        if (!strcmp(Entry->Value, "1"))
        {
            IncludeList.Add(Entry->Entry);
        }
    }

    ExcludeList.Clear();
    INISection* ExcludeSection = INI->Get_Section("Excludes");
    for (INIEntry* Entry = ExcludeSection->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next())
    {
        if (!strcmp(Entry->Value, "1"))
        {
            ExcludeList.Add(Entry->Entry);
        }
    }
}

void UP_Building_Warnings::Damage_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type)
{
    StringClass Preset = Commands->Get_Preset_Name(Victim);
    if (ExcludeList.ID(Preset) == -1 && (Victim->As_BuildingGameObj() || IncludeList.ID(Preset) > -1 || (ReportVehicleBuildings && Victim->As_VehicleGameObj() && Victim->As_VehicleGameObj()->Get_Definition().EncyclopediaType == 3)))
    {
        float Previous = Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim) + Damage;
        float Total = Commands->Get_Health(Victim) + Commands->Get_Shield_Strength(Victim);
        float Max = Commands->Get_Max_Health(Victim) + Commands->Get_Max_Shield_Strength(Victim);
        
        int Team = Get_Object_Type(Victim);
        StringClass Format;

        FormatParams *Params = new FormatParams;
        Params->Building = Victim;
        Params->Damager = Damager;
        Params->Team = Team;
        Params->Out = &Format;
        
        if (Total > 0)
        {
            if (Damage > 0)
            {
                if (Previous / Max > 0.25f && Total / Max < 0.25f)
                {
                    if (!Format25.Is_Empty())
                    {
                        Params->Message = Format25;
                        Params->State = WarnState::WARNSTATE_25;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max > 0.50f && Total / Max < 0.50f)
                {
                    if (!Format50.Is_Empty())
                    {
                        Params->Message = Format50;
                        Params->State = WarnState::WARNSTATE_50;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max > 0.75f && Total / Max < 0.75f)
                {
                    if (!Format75.Is_Empty())
                    {
                        Params->Message = Format75;
                        Params->State = WarnState::WARNSTATE_75;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }
            }
            else if (Damage < 0)
            {
                if (Previous / Max < 0.25f && Total / Max > 0.25f)
                {
                    if (!Format25.Is_Empty())
                    {
                        Params->Message = Format25;
                        Params->State = WarnState::WARNSTATE_25;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max < 0.50f && Total / Max > 0.50f)
                {
                    if (!Format50.Is_Empty())
                    {
                        Params->Message = Format50;
                        Params->State = WarnState::WARNSTATE_50;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max < 0.75f && Total / Max > 0.75f)
                {
                    if (!Format75.Is_Empty())
                    {
                        Params->Message = Format75;
                        Params->State = WarnState::WARNSTATE_75;

                        FormatStateMessage(Params, Format);
                        SendStateMessage(Params);
                    }
                }

                if (!FormatRepairing.Is_Empty())
                {
                    Params->Message = FormatRepairing;
                    Params->State = WarnState::WARNSTATE_REPAIRING;

                    FormatStateMessage(Params, Format);
                    SendStateMessage(Params);
                }
            }
        }

        delete Params->Message;
        delete Params;
        delete Format;
    }

    delete Preset;
}

void UP_Building_Warnings::SendStateMessage(FormatParams *Params)
{
    if (!Is_Timer(static_cast<int>(Params->State), Commands->Get_ID(Params->Building)))
    {
        Vector3 Color;

        if (UseTeamColors)
        {
            Color = Get_Color_For_Team(Params->Team);

            Color.X *= 255.0f;
            Color.Y *= 255.0f;
            Color.Z *= 255.0f;
        }
        else
        {
            switch (Params->State)
            {
                case WarnState::WARNSTATE_75:
                    Color = Color75;
                    break;
                case WarnState::WARNSTATE_50:
                    Color = Color50;
                    break;
                case WarnState::WARNSTATE_25:
                    Color = Color25;
                    break;
                case WarnState::WARNSTATE_REPAIRING:
                    Color = ColorRepairing;
                    break;
            }
        }

        switch (Params->State)
        {
        case WarnState::WARNSTATE_75:
            DA::Create_2D_Sound_Team(Params->Team, Sounds[0][Params->Team]);
            if(ShowEnemy) DA::Create_2D_Sound_Team(PTTEAM(Params->Team), Sounds[0][Params->Team]);
            break;
        case WarnState::WARNSTATE_50:
            DA::Create_2D_Sound_Team(Params->Team, Sounds[1][Params->Team]);
            if (ShowEnemy) DA::Create_2D_Sound_Team(PTTEAM(Params->Team), Sounds[1][Params->Team]);
            break;
        case WarnState::WARNSTATE_25:
            DA::Create_2D_Sound_Team(Params->Team, Sounds[2][Params->Team]);
            if (ShowEnemy) DA::Create_2D_Sound_Team(PTTEAM(Params->Team), Sounds[2][Params->Team]);
            break;
        case WarnState::WARNSTATE_REPAIRING:
            DA::Create_2D_Sound_Team(Params->Team, Sounds[3][Params->Team]);
            break;
        }

        DA::Team_Color_Message(Params->Team, (int)Color.X, (int)Color.Y, (int)Color.Z, "%s%s", UseDAPrefix ? DA::Get_Message_Prefix() : "", (*Params->Out));

        if (ShowEnemy && Params->State != WarnState::WARNSTATE_REPAIRING)
        {
            DA::Team_Color_Message(PTTEAM(Params->Team), (int)Color.X, (int)Color.Y, (int)Color.Z, "%s%s", UseDAPrefix ? DA::Get_Message_Prefix() : "", (*Params->Out));
        }

        Cooldown(Params->State, Commands->Get_ID(Params->Building));
    }
}

void UP_Building_Warnings::FormatStateMessage(FormatParams *Params, StringClass &Out)
{
    (*Params->Out) += Params->Message;
    Params->Out->Replace("{TEAM}", Get_Team_Name(Params->Team));
    Params->Out->Replace("{NAME}", DATranslationManager::Translate(Params->Building));
    Params->Out->Replace("{PLAYER}", Get_Player_Name(Params->Damager));
}

void UP_Building_Warnings::Timer_Expired(int Number, unsigned int Data)
{
    // hmm
}

UP_Building_Warnings::~UP_Building_Warnings()
{
    if (INI)
        delete INI;
}

Register_Game_Feature(UP_Building_Warnings, "Building Warnings", "EnableBuildingWarnings", 0);