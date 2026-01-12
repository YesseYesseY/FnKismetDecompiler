#include <Windows.h>
#include <iostream>
#include <fstream>
#include <print>
#include <vector>
#include <unordered_map>

#include "UnrealCore.hpp"
using namespace UnrealCore;

#define MessageBox(...) MessageBoxA(NULL, std::format(__VA_ARGS__).c_str(), "KismetDecompiler", MB_OK)

// #define SEARCH_FOR_UNKNOWNS
#include "KismetDisassembler.hpp"
#include "KismetDecompiler.hpp"

DWORD MainThread(HMODULE Module)
{
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    InitUnrealCore();

    // Just took a random ExecuteUbergraph
    // auto Func = UObject::FindFunction(L"/Game/Athena/SupplyDrops/BP_DamageBalloon_Athena.BP_DamageBalloon_Athena_C:ExecuteUbergraph_BP_DamageBalloon_Athena");
    // auto Class = UObject::FindClass(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
    // auto BlueprintGeneratedClassClass = UObject::FindClass(L"/Script/Engine.BlueprintGeneratedClass");

#ifdef SEARCH_FOR_UNKNOWNS
#if 1
    auto SystemLib = UObject::FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
    struct {
        UObject* WorldContext;
        FString Cmd;
        UObject* Player;
    } args { UObject::FindObject(L"/Game/Maps/Frontend.Frontend"), L"open Athena_Terrain" };
    SystemLib->ProcessEvent("ExecuteConsoleCommand", &args);

    MessageBox("Click OK to search for unknowns");
#endif

    for (int i = 0; i < UObject::Objects->Num(); i++)
    {
        auto Object = UObject::Objects->GetObject(i);
        if (Object && Object->IsA(BlueprintGeneratedClassClass))
        {
            auto Class = (UClass*)Object;
            KismetDisassembler().Disassemble(Class);
        }
    }
    MessageBox("No unknowns!");
#else
#if 0 // Disassemble all BlueprintGeneratedClasses
    for (int i = 0; i < UObject::Objects->Num(); i++)
    {
        auto Object = UObject::Objects->GetObject(i);
        if (Object && Object->IsA(BlueprintGeneratedClassClass))
        {
            auto Class = (UClass*)Object;
            std::ofstream outfile("scripts/" + Class->GetName() + ".txt");
            outfile << KismetDisassembler().Disassemble(Class);
            outfile.close();
        }
    }
#else
    // auto Class = UObject::FindClass(L"/Game/Athena/Athena_PlayerController.Athena_PlayerController_C");
    auto Class = UObject::FindClass(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/SupplyDrops/BP_DamageBalloon_Athena.BP_DamageBalloon_Athena_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/DrivableVehicles/Mech/TestMechVehicle.TestMechVehicle_C");
#if 0 // Disassemble or Decompile
    std::ofstream outfile("script.txt");
    outfile << KismetDisassembler().Disassemble(Class);
    outfile.close();
#else
    std::ofstream outfile("decomp/" + Class->GetName() + ".cpp");
    outfile << KismetDecompiler().Disassemble(Class);
    outfile.close();

#endif
#endif
#endif

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    }

    return TRUE;
}
