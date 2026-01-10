#include <Windows.h>
#include <iostream>
#include <fstream>
#include <print>

#include "UnrealCore.hpp"
using namespace UnrealCore;

#define MessageBox(...) MessageBoxA(NULL, std::format(__VA_ARGS__).c_str(), "KismetDecompiler", MB_OK)

#include "KismetDisassembler.hpp"

DWORD MainThread(HMODULE Module)
{
    AllocConsole();
    FILE* Dummy;
    freopen_s(&Dummy, "CONOUT$", "w", stdout);
    freopen_s(&Dummy, "CONIN$", "r", stdin);

    InitUnrealCore();

    // Just took a random ExecuteUbergraph
    auto Func = UObject::FindFunction(L"/Game/Athena/SupplyDrops/BP_DamageBalloon_Athena.BP_DamageBalloon_Athena_C:ExecuteUbergraph_BP_DamageBalloon_Athena");

    std::ofstream outfile("script.txt");
    outfile << KismetDisassembler(Func).Disassemble();
    outfile.close();

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
