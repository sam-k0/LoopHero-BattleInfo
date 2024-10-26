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
#define _CRT_SECURE_NO_WARNINGS


CallbackAttributes_t* CodeCallbackAttr;

// Unload function, remove callbacks here
YYTKStatus PluginUnload()
{
    LHCore::pUnregisterModule(gPluginName);
    return YYTK_OK;
}

int CodePrePatch(YYTKCodeEvent* codeEvent, void*)
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

        // Get the enemy hp
        YYRValue hp = Binds::CallBuiltinA("variable_instance_get", { instid, "hp" });
        // draw the hp
        Misc::Print(std::to_string(hp.As<double>()));

    }

    return YYTK_OK;

}


void InstallPatches() // Register Pre and Post patches here
{
	if (LHCore::pInstallPrePatch != nullptr)
	{
		LHCore::pInstallPrePatch(CodePrePatch);
        Misc::Print("Installed patch method(s)", CLR_GREEN);
	}
	else
	{
		Misc::Print("PrePatch not found");
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

