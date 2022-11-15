/*
    Building Warnings
    Copyright 2020 Unstoppable

    This plugin is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. See GNU General Public License for more details.
*/

#include "General.h"
//Engine
#include "engine_io.h"
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
#define ColorCast(Color) static_cast<int>(Color)

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

    Enable75 = true;
    Enable50 = true;
    Enable25 = true;
    EnableRepairing = true;

    IncludeList = DynamicVectorClass<StringClass>();
    ExcludeList = DynamicVectorClass<StringClass>();

    if (INI)
        delete INI;

    INI = new DASettingsClass("BuildingWarnings.ini");

    //Events

    Register_Event(DAEvent::SETTINGSLOADED);
    Register_Object_Event(DAObjectEvent::DAMAGERECEIVED, DAObjectEvent::ALL, INT_MAX);

    Console_Output("%sBuilding Warnings successfully initialized!\n", DA::Get_Message_Prefix());
    Console_Output("%sVersion %.2f by Unstoppable, built at %s\n", DA::Get_Message_Prefix(), Version, CompileTime);
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

    Enable75 = INI->Get_Bool("Enable", "Percent75", true);
    Enable50 = INI->Get_Bool("Enable", "Percent50", true);
    Enable25 = INI->Get_Bool("Enable", "Percent25", true);
    EnableRepairing = INI->Get_Bool("Enable", "Repairing", true);

    IncludeList.Clear();
    INISection* IncludeSection = INI->Get_Section("Includes");
    if (IncludeSection)
    {
        for (INIEntry* Entry = IncludeSection->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next())
        {
            if (!strcmp(Entry->Value, "1"))
            {
                IncludeList.Add(Entry->Entry);
            }
        }
    }

    ExcludeList.Clear();
    INISection* ExcludeSection = INI->Get_Section("Excludes");
    if (ExcludeSection)
    {
        for (INIEntry* Entry = ExcludeSection->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next())
        {
            if (!strcmp(Entry->Value, "1"))
            {
                ExcludeList.Add(Entry->Entry);
            }
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
                    if (Enable25)
                    {
                        Params->Message = Format25;
                        Params->State = WarnState::WARNSTATE_25;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max > 0.50f && Total / Max < 0.50f)
                {
                    if (Enable50)
                    {
                        Params->Message = Format50;
                        Params->State = WarnState::WARNSTATE_50;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max > 0.75f && Total / Max < 0.75f)
                {
                    if (Enable75)
                    {
                        Params->Message = Format75;
                        Params->State = WarnState::WARNSTATE_75;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }
            }
            else if (Damage < 0)
            {
                if (Previous / Max < 0.25f && Total / Max > 0.25f)
                {
                    if (Enable25)
                    {
                        Params->Message = Format25;
                        Params->State = WarnState::WARNSTATE_25;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max < 0.50f && Total / Max > 0.50f)
                {
                    if (Enable50)
                    {
                        Params->Message = Format50;
                        Params->State = WarnState::WARNSTATE_50;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }
                else if (Previous / Max < 0.75f && Total / Max > 0.75f)
                {
                    if (Enable75)
                    {
                        Params->Message = Format75;
                        Params->State = WarnState::WARNSTATE_75;

                        FormatStateMessage(Params);
                        SendStateMessage(Params);
                    }
                }

                if (EnableRepairing)
                {
                    Params->Message = FormatRepairing;
                    Params->State = WarnState::WARNSTATE_REPAIRING;

                    FormatStateMessage(Params);
                    SendStateMessage(Params);
                }
            }
        }

        delete Params;
    }
}

void UP_Building_Warnings::SendStateMessage(FormatParams *Params)
{
    if (!Is_Timer(ColorCast(Params->State), Commands->Get_ID(Params->Building)))
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

        DA::Team_Color_Message(Params->Team, ColorCast(Color.X), ColorCast(Color.Y), ColorCast(Color.Z), "%s%s", UseDAPrefix ? DA::Get_Message_Prefix() : "", (*Params->Out));

        if (ShowEnemy && Params->State != WarnState::WARNSTATE_REPAIRING)
        {
            DA::Team_Color_Message(PTTEAM(Params->Team), ColorCast(Color.X), ColorCast(Color.Y), ColorCast(Color.Z), "%s%s", UseDAPrefix ? DA::Get_Message_Prefix() : "", (*Params->Out));
        }

        Cooldown(Params->State, Commands->Get_ID(Params->Building));
    }
}

void UP_Building_Warnings::FormatStateMessage(FormatParams *Params)
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