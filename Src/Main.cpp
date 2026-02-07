#include <Windows.h>
#include <iostream>
#include <fstream>
#include <print>
#include <vector>
#include <unordered_map>

#include "UnrealCore.hpp"
using namespace UnrealCore;

#define MessageBox(...) MessageBoxA(NULL, std::format(__VA_ARGS__).c_str(), "KismetDecompiler", MB_OK)

#define BR_MAP L"Athena_Terrain"

#define SEARCH_FOR_UNKNOWNS 0
#define LOAD_BR_MAP 1
#define DECOMP_ALL_BLUEPRINTS 1
#define DUMP_OBJECTS 0
#define PROCESS_DATA_ASSETS 0 // Warning: Very slow! If you can use FModel just use that instead

#include "KismetDisassembler.hpp"
#include "KismetDecompiler.hpp"
#include "Utils.hpp"

DWORD MainThread(HMODULE Module)
{
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    InitUnrealCore();

    auto BlueprintGeneratedClassClass = UObject::FindClass(L"/Script/Engine.BlueprintGeneratedClass");
    auto PrimaryDataAssetClass = UObject::FindClass(L"/Script/Engine.PrimaryDataAsset");

    // MessageBox("{}", EngineVersion);

#if LOAD_BR_MAP
    auto SystemLib = UObject::FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
    struct {
        UObject* WorldContext;
        FString Cmd;
        UObject* Player;
    } args { UObject::FindObject(L"/Game/Maps/Frontend.Frontend"), L"open " BR_MAP };
    SystemLib->ProcessEvent("ExecuteConsoleCommand", &args);

    MessageBox("Click OK when fully loaded into the map");
#endif

#if DUMP_OBJECTS
    Utils::DumpObjects();
#endif

#if SEARCH_FOR_UNKNOWNS
    MessageBox("Click OK to search for unknowns");

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
#if DECOMP_ALL_BLUEPRINTS // Disassemble all BlueprintGeneratedClasses
    for (int i = 0; i < UObject::Objects->Num(); i++)
    {
        auto Object = UObject::Objects->GetObject(i);

        if (!Object)
            continue;

        if (Object->IsA(BlueprintGeneratedClassClass))
        {
            auto Class = (UClass*)Object;
            std::ofstream outfile("decomp/" + Class->GetNameSafe() + ".cpp");
            outfile << KismetDecompiler().Disassemble(Class);
            outfile.close();
        }
#if PROCESS_DATA_ASSETS
        else if (Object->IsA(PrimaryDataAssetClass))
        {
            std::ofstream outfile("data/" + Object->GetNameSafe() + ".cpp");
            outfile << KismetDecompiler().ProcessDataAsset(Object);
            outfile.close();
        }
#endif
    }
    MessageBox("Finished decompiling all blueprints");
#else
    auto Class = UObject::FindClass(L"/Game/Athena/Athena_PlayerController.Athena_PlayerController_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/PlayerPawn_Athena.PlayerPawn_Athena_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/B_AthenaAlwaysLoadedContentHack.B_AthenaAlwaysLoadedContentHack_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/SupplyDrops/BP_DamageBalloon_Athena.BP_DamageBalloon_Athena_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/DrivableVehicles/Mech/TestMechVehicle.TestMechVehicle_C");
    // auto Class = UObject::FindClass(L"/Game/Athena/Items/EnvironmentalItems/ExplosiveProps/Apollo_GasPump_Valet.Apollo_GasPump_Valet_C");

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
