#include "UnrealContainers.hpp"
using namespace UC;

#include "UnrealCoreEnums.hpp"
#include "Memcury.hpp"

#define UnrealMessageBox(...) MessageBoxA(NULL, std::format(__VA_ARGS__).c_str(), "UnrealCore", MB_OK)
#define CheckAddr(FailMsg) if (!Addr) { UnrealMessageBox(FailMsg); return; }

namespace UnrealCore
{
    namespace Offsets
    {
        static int32 UObject_NamePrivate = -1;
        static int32 UObject_Class = -1;
        static int32 UObject_ObjectFlags = -1;

        static int32 UField_Next = -1;

        static int32 UStruct_SuperStruct = -1;
        static int32 UStruct_Children = -1;
        static int32 UStruct_Size = -1;
        static int32 UStruct_Script = -1;

        static int32 UProperty_Offset = -1;

        static int32 UFunction_ExecFunc = -1;
    }

    namespace UnrealOptions
    {
        static bool ChunkedObjectArray = false;
    }

    static inline float EngineVersion = -1.0f;
    static inline float GameVersion = -1.0f;

    class FName
    {
    public:
        int32 ComparisonIndex;
        int32 Number;

    public:
        std::string ToString();
        std::wstring ToWString();
    };

    // Shouldn't change? atleast on the ue versions that matter
    struct FUObjectItem
    {
        class UObject* Object;
        int32 Flags;
        int32 ClusterRootIndex;
        int32 SerialNumber;
    };

    class UObjectArray
    {
    private:
        FUObjectItem* Fixed_GetObjects()
        {
            return *(FUObjectItem**)(int64(this));
        }

        int32 Fixed_MaxElements()
        {
            return *(int32*)(int64(this) + 8);
        }

        int32 Fixed_NumElements()
        {
            return *(int32*)(int64(this) + 12);
        }

        FUObjectItem* Fixed_GetObjectItem(int32 Index)
        {
            if (Index >= 0 && Index < Fixed_NumElements())
                return &Fixed_GetObjects()[Index];

            return nullptr;
        }

    private:
        FUObjectItem** Chunked_GetObjects()
        {
            return *(FUObjectItem***)(int64(this));
        }

        int32 Chunked_MaxElements()
        {
            return *(int32*)(int64(this) + 16);
        }

        int32 Chunked_NumElements()
        {
            return *(int32*)(int64(this) + 20);
        }

        int32 Chunked_MaxChunks()
        {
            return *(int32*)(int64(this) + 24);
        }

        int32 Chunked_NumChunks()
        {
            return *(int32*)(int64(this) + 28);
        }

        enum
        {
            NumElementsPerChunk = 64 * 1024,
        };

        FUObjectItem* Chunked_GetObjectItem(int32 Index)
        {
            const int32 ChunkIndex = Index / NumElementsPerChunk;
            const int32 WithinChunkIndex = Index % NumElementsPerChunk;
            if (Index < 0 || Index >= Chunked_NumElements())
                return nullptr;

            if (ChunkIndex >= Chunked_NumChunks())
                return nullptr;

            if (Index >= Chunked_MaxElements())
                return nullptr;

            FUObjectItem* Chunk = Chunked_GetObjects()[ChunkIndex];
            if (!Chunk)
                return nullptr;

            return &Chunk[WithinChunkIndex];
        }

    public:
        int32 Num()
        {
            if (UnrealOptions::ChunkedObjectArray)
                return Chunked_NumElements();

            return Fixed_NumElements();
        }

        FUObjectItem* GetObjectItem(int32 Index)
        {
            if (UnrealOptions::ChunkedObjectArray)
                return Chunked_GetObjectItem(Index);

            return Fixed_GetObjectItem(Index);
        }

        class UObject* GetObject(int32 Index)
        {
            if (auto ObjItem = GetObjectItem(Index))
                return ObjItem->Object;

            return nullptr;
        }
    };

    class UObject
    {
    private:
        static inline UObject* (*StaticFindObject)(class UClass*, UObject*, const wchar_t*, bool) = nullptr;
        static inline void (*ProcessEventNative)(UObject*, class UFunction*, void*) = nullptr;

    public:
        static inline UObjectArray* Objects = nullptr;

    public:
        static void Init()
        {
            // StaticFindObject
            {
                // ScanFor bytes will probably change
                auto Addr = Memcury::Scanner::FindStringRef(L"Illegal call to StaticFindObject() while serializing object data!").ScanFor({ 0x48, 0x89, 0x5C }, false).Get();

                CheckAddr("Failed to find StaticFindObject");

                StaticFindObject = decltype(StaticFindObject)(Addr);
            }

            // ProcessEvent
            {
                auto Addr = Memcury::Scanner::FindPattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC F0 00 00 00").Get();

                CheckAddr("Failed to find ProcessEvent");

                ProcessEventNative = decltype(ProcessEventNative)(Addr);
            }

            // Objects
            {
                // 4.1
                auto Addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 0C 49 48 8D 14 C8 EB ? 48 8B D3 8B 42 ? C1 E8 1D A8 01 74").RelativeOffset(3).Get();

                if (!Addr)
                    Addr = Memcury::Scanner::FindStringRef(L"SubmitRemoteVoiceData(%s) Size: %d received!").ScanFor({ 0x48, 0x8B, 0x05 }, true, 1).RelativeOffset(3).Get();

                CheckAddr("Failed to find Objects");

                Objects = (UObjectArray*)(Addr);
            }
        }

        static UObject* FindObject(const wchar_t* Name)
        {
            return StaticFindObject(nullptr, nullptr, Name, false);
        }

        static class UClass* FindClass(const wchar_t* Name)
        {
            static auto ClassClass = (UClass*)FindObject(L"/Script/CoreUObject.Class");
            return (UClass*)StaticFindObject(ClassClass, nullptr, Name, false); 
        }

        static class UFunction* FindFunction(const wchar_t* Name)
        {
            static auto FunctionClass = FindClass(L"/Script/CoreUObject.Function");
            return (UFunction*)StaticFindObject(FunctionClass, nullptr, Name, true); 
        }

        // This ignores default objects
        static UObject* FindFirstObjectOfClass(class UClass* ObjectClass);

    public:
        void** VTable;

    public:
        int32 GetChildOffset(const std::string& Name);

        template <typename T = UObject*>
        T* GetChildPtr(int32 Offset)
        {
            return (T*)(int64(this) + Offset);
        }

        template <typename T = UObject*>
        T& GetChild(int32 Offset)
        {
            return *GetChildPtr<T>(Offset);
        }

        template <typename T = UObject*>
        T* GetChildPtr(const std::string& Name)
        {
            return GetChildPtr<T>(GetChildOffset(Name));
        }

        template <typename T = UObject*>
        T& GetChild(const std::string& Name)
        {
            return GetChild<T>(GetChildOffset(Name));
        }

        FName& GetNamePrivate()
        {
            return GetChild<FName>(Offsets::UObject_NamePrivate);
        }

        class UClass* GetClass()
        {
            return GetChild<class UClass*>(Offsets::UObject_Class);
        }

        EObjectFlags GetObjectFlags()
        {
            return GetChild<EObjectFlags>(Offsets::UObject_ObjectFlags);
        }

        UFunction* GetFunction(const std::string& Name);

    public:
        void ProcessEvent(class UFunction* Function, void* Args = nullptr)
        {
            ProcessEventNative(this, Function, Args);
        }

        void ProcessEvent(const std::string& FunctionName, void* Args = nullptr)
        {
            if (auto Function = GetFunction(FunctionName))
            {
                ProcessEventNative(this, Function, Args);
            }
        }

        std::string GetName()
        {
            if (this)
                return GetNamePrivate().ToString();

            return "None";
        }

        std::wstring GetWName()
        {
            return GetNamePrivate().ToWString();
        }

        bool HasObjectFlag(EObjectFlags Flag)
        {
            return (GetObjectFlags() & Flag) != EObjectFlags::RF_NoFlags;
        }

        bool IsDefaultObject()
        {
            return HasObjectFlag(EObjectFlags::RF_ClassDefaultObject);
        }

        std::string GetFullName();

        bool IsA(class UClass* Other);
    };

    class UField : public UObject
    {
    public:
        UField* GetNext()
        {
            return GetChild<UField*>(Offsets::UField_Next);
        }
    };

    class UProperty : public UField
    {
    public:
        int32 GetOffset()
        {
            return GetChild<int32>(Offsets::UProperty_Offset);
        }
    };

    class UStruct : public UField
    {
    public:
        UStruct* GetSuperStruct()
        {
            return GetChild<UStruct*>(Offsets::UStruct_SuperStruct);
        }

        UField* GetChildren()
        {
            return GetChild<UField*>(Offsets::UStruct_Children);
        }

        int32 GetSize()
        {
            return GetChild<int32>(Offsets::UStruct_Size);
        }

        TArray<uint8> GetScript()
        {
            return GetChild<TArray<uint8>>(Offsets::UStruct_Script);
        }

    public:
        bool IsChildOf(UStruct* Other)
        {
            for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
            {
                if (Struct == Other)
                {
                    return true;
                }
            }

            return false;
        }

        int32 GetChildOffset(const std::string& Name)
        {
            static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");
            for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
            {
                for (auto Child = Struct->GetChildren(); Child; Child = Child->GetNext())
                {
                    if (Child->IsA(FunctionClass))
                        continue;

                    if (Child->GetName() == Name)
                        return ((UProperty*)Child)->GetOffset();
                }
            }

            return -1;
        }

        UFunction* GetChildFunction(const std::string& Name)
        {
            static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");
            for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
            {
                for (auto Child = Struct->GetChildren(); Child; Child = Child->GetNext())
                {
                    if (!Child->IsA(FunctionClass))
                        continue;

                    if (Child->GetName() == Name)
                        return (UFunction*)Child;
                }
            }

            return nullptr;
        }
    };

    class UClass : public UStruct
    {
    };

    class UFunction : public UStruct
    {
    public:
        void* GetExecFunc()
        {
            return GetChild<void*>(Offsets::UFunction_ExecFunc);
        }
    };

    std::string FName::ToString()
    {
        static auto StringLib = UObject::FindObject(L"/Script/Engine.Default__KismetStringLibrary");
        static auto Func = UObject::FindFunction(L"/Script/Engine.KismetStringLibrary.Conv_NameToString");
        struct {
            FName InName;
            FString ReturnValue;
        } args { *this };
        StringLib->ProcessEvent(Func, &args);
        auto ret = args.ReturnValue.ToString();
        args.ReturnValue.Free();
        return ret;
    }

    std::wstring FName::ToWString()
    {
        static auto StringLib = UObject::FindObject(L"/Script/Engine.Default__KismetStringLibrary");
        static auto Func = UObject::FindFunction(L"/Script/Engine.KismetStringLibrary.Conv_NameToString");
        struct {
            FName InName;
            FString ReturnValue;
        } args { *this };
        StringLib->ProcessEvent(Func, &args);
        auto ret = args.ReturnValue.ToWString();
        args.ReturnValue.Free();
        return ret;
    }

    std::string UObject::GetFullName()
    {
        if (!this)
            return "None";

        static auto StringLib = UObject::FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
        static auto Func = UObject::FindFunction(L"/Script/Engine.KismetSystemLibrary.GetPathName");
        struct {
            UObject* Object;
            FString ReturnValue;
        } args { this };
        StringLib->ProcessEvent(Func, &args);
        auto ret = std::format("{} {}", GetClass()->GetName(), args.ReturnValue.ToString());
        args.ReturnValue.Free();
        return ret;
    }

    bool UObject::IsA(class UClass* Other)
    {
        if (auto Class = GetClass())
            return Class->IsChildOf(Other);

        return false;
    }

    int32 UObject::GetChildOffset(const std::string& Name)
    {
        return GetClass()->GetChildOffset(Name);
    }

    UObject* UObject::FindFirstObjectOfClass(class UClass* ObjectClass)
    {
        for (int i = 0; i < Objects->Num(); i++)
        {
            auto Object = Objects->GetObject(i);
            if (!Object || Object->IsDefaultObject())
                continue;

            if (Object->IsA(ObjectClass))
                return Object;
        }

        return nullptr;
    }

    UFunction* UObject::GetFunction(const std::string& Name)
    {
        return GetClass()->GetChildFunction(Name);
    }



    static void InitUnrealCore()
    {
        UObject::Init();

        // FMemory::Realloc
        {
            // 4.1
            auto Addr = Memcury::Scanner::FindPattern("4C 8B D1 48 8B 0D ? ? ? ? 48 85 C9 75 ? 49 8B CA").GetAs<void*>();

            if (!Addr) // 7.30
                Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 41 8B D8 48 8B 0D").GetAs<void*>();

            CheckAddr("Failed to find FMemory::Realloc");

            FMemory::Init(Addr);
        }

        // EngineVersion and GameVersion
        {
            static auto SystemLib = UObject::FindObject(L"/Script/Engine.Default__KismetSystemLibrary");
            static auto Func = UObject::FindFunction(L"/Script/Engine.KismetSystemLibrary.GetEngineVersion");
            FString Ver;
            SystemLib->ProcessEvent(Func, &Ver);
            auto VerStr = Ver.ToString();
            Ver.Free();
            EngineVersion = std::stof(VerStr);
            GameVersion = std::stof(VerStr.substr(VerStr.find_last_of('-') + 1));
        }

        Offsets::UObject_ObjectFlags = 0x8;
        Offsets::UObject_Class = 0x10;
        Offsets::UObject_NamePrivate = 0x18;

        Offsets::UField_Next = 0x28;

        Offsets::UStruct_SuperStruct = 0x30;
        if (EngineVersion >= 4.22f) // TODO Check 4.21
            Offsets::UStruct_SuperStruct += 0x10;

        Offsets::UStruct_Children = Offsets::UStruct_SuperStruct + 0x8;
        Offsets::UStruct_Size = Offsets::UStruct_Children + 0x8;
        Offsets::UStruct_Script = Offsets::UStruct_Size + 0x8;

        Offsets::UProperty_Offset = 0x44;

        Offsets::UFunction_ExecFunc = 0xB0;
        if (EngineVersion >= 4.22f) // From UStruct
            Offsets::UFunction_ExecFunc += 0x10;

        UnrealOptions::ChunkedObjectArray = EngineVersion >= 4.22f; // TODO Check 4.21
    }
}
