// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>

// YYTK is in this now
#include "MyPlugin.h"
#include "Assets.h"
#include "LHSprites.h"
// Plugin functionality
#include <fstream>
#include <iterator>
#include "LHCore.h"
#include "Config.h"
#define _CRT_SECURE_NO_WARNINGS


CallbackAttributes_t* CodeCallbackAttr;
std::string cfgFilename = Filesys::GetCurrentDir() + "\\options.ini";
std::string SectionName = "BattleInfo";
std::string KeyNameDecPrecision = "DecPrecision";
std::string KeyNameTextSize = "TextSize";
std::string KeyNameTextOffsetX = "TextOffsetX";
std::string KeyNameTextOffsetY = "TextOffsetY";


int DecPrecision = 2;
double TextSize = 0.5;
int TextOffsetX = 0;
int TextOffsetY = 0;

// Unload function, remove callbacks here
YYTKStatus PluginUnload()
{
    LHCore::pUnregisterModule(gPluginName);
    return YYTK_OK;
}

int CodePrePatch(YYTKCodeEvent* codeEvent, void*)
{
	return YYTK_OK;
}

int CodePostPatch(YYTKCodeEvent* codeEvent, void*)
{
    
    CCode* codeObj = std::get<CCode*>(codeEvent->Arguments());
    CInstance* selfInst = std::get<0>(codeEvent->Arguments());
    CInstance* otherInst = std::get<1>(codeEvent->Arguments());


    // If we have invalid data???
    if (!codeObj)
        return YYTK_INVALIDARG;

    if (!codeObj->i_pName)
        return YYTK_INVALIDARG;

    // Do event specific stuff here.
    if ((Misc::StringHasSubstr(codeObj->i_pName, "o_fight_enemy_Draw_0")))
    {
        double instid = selfInst->i_spriteindex;

        if(false)
		{
            YYRValue vars;
            Binds::GetInstanceVariables(vars, instid);
            Binds::PrintArrayInstanceVariables(vars, instid);
		}

        // Get the enemy hp
        YYRValue hp = Binds::CallBuiltinA("variable_instance_get", { instid, "hp" });
		YYRValue atkpersec = Binds::CallBuiltinA("variable_instance_get", { instid, "spd" });
		YYRValue atk = Binds::CallBuiltinA("variable_instance_get", { instid, "str" });

        // Calc Dps
		double dps = atk.As<double>() * atkpersec.As<double>();

		// get x and y of the enemy
		YYRValue x = Binds::CallBuiltinA("variable_instance_get", { instid, "x" });
		YYRValue y = Binds::CallBuiltinA("variable_instance_get", { instid, "y" });
		YYRValue spr = Binds::CallBuiltinA("variable_instance_get", { instid, "sprite_index" });
		YYRValue sprwidth = Binds::CallBuiltinA("sprite_get_width", { spr });

        // Get the current font and drawing settings like valign and halign
		YYRValue font = Binds::CallBuiltinA("draw_get_font", {});
		YYRValue halign = Binds::CallBuiltinA("draw_get_halign", {});

		// Set the font to the default font
		Binds::CallBuiltinA("draw_set_font", { 1. });
		Binds::CallBuiltinA("draw_set_halign", { 1. });

        double sprwd = sprwidth.As<double>();
        // assemble string 
		std::string info = "HP: " + Misc::to_string_trimmed(hp.As<double>(), DecPrecision) + " DPS: " + Misc::to_string_trimmed(dps, DecPrecision);
		Binds::CallBuiltinA("draw_text_transformed", { 
            x.As<double>() + sprwd / 2 + TextOffsetX,
            y.As<double>() + TextOffsetY,
            info,
            TextSize,
            TextSize,
            0.});

		// Reset the font and drawing settings
		Binds::CallBuiltinA("draw_set_font", { font });
        Binds::CallBuiltinA("draw_set_halign", { halign });
    }
    return YYTK_OK;
}


void InstallPatches() // Register Pre and Post patches here
{
	if (LHCore::pInstallPostPatch == nullptr || LHCore::pInstallPrePatch == nullptr)
	{
		Misc::PrintDbg("Failed to install patches",__FUNCTION__, __LINE__ ,CLR_RED);
		return;
	}
    else
	{
		LHCore::pInstallPrePatch(CodePrePatch);
		LHCore::pInstallPostPatch(CodePostPatch);
        Misc::Print("Installed patch method(s)", CLR_GREEN);
	}


    // Write default config values
    if (Filesys::FileExists(cfgFilename))
    {
		if (Config::KeySectionExists(cfgFilename, SectionName, KeyNameDecPrecision)) {
			// Read the value
			Misc::PrintDbg("Reading config values", __FUNCTION__, __LINE__, CLR_GREEN);
            DecPrecision = Config::ReadIntFromIni(cfgFilename, SectionName, KeyNameDecPrecision, 2);
            TextOffsetX = Config::ReadIntFromIni(cfgFilename, SectionName, KeyNameTextOffsetX, 0);
            TextOffsetY = Config::ReadIntFromIni(cfgFilename, SectionName, KeyNameTextOffsetY, 0);
            TextSize = Config::ReadDoubleFromIni(cfgFilename, SectionName, KeyNameTextSize, 0.5);

            Misc::Print("Read text size: " + std::to_string(TextSize));
		}
		else
		{
			// Write a default value
            Misc::PrintDbg("Writing default values to config", __FUNCTION__, __LINE__, CLR_GRAY);
			Config::WriteIniValue(cfgFilename, SectionName, KeyNameDecPrecision, std::to_string(DecPrecision));
            Config::WriteIniValue(cfgFilename, SectionName, KeyNameTextOffsetX, std::to_string(TextOffsetX));
            Config::WriteIniValue(cfgFilename, SectionName, KeyNameTextOffsetY, std::to_string(TextOffsetY));
			Config::WriteIniValue(cfgFilename, SectionName, KeyNameTextSize, std::to_string(TextSize));
		}
    }
    
}

// Entry
DllExport YYTKStatus PluginEntry(
    YYTKPlugin* PluginObject // A pointer to the dedicated plugin object
)
{
    LHCore::CoreReadyPack* pack = new LHCore::CoreReadyPack(PluginObject, InstallPatches);
    PluginObject->PluginUnload = PluginUnload;
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LHCore::ResolveCore, (LPVOID)pack, 0, NULL); // Wait for LHCC
    return YYTK_OK; // Successful PluginEntry.
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DllHandle = hModule; // save our module handle
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

