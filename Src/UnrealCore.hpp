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
        static int32 UObject_Outer = -1;

        static int32 UField_Next = -1;

        static int32 UStruct_SuperStruct = -1;
        static int32 UStruct_Children = -1;
        static int32 UStruct_Size = -1;
        static int32 UStruct_Script = -1;
        static int32 UStruct_ChildProperties = -1;

        static int32 UProperty_Offset = -1;
        static int32 UProperty_PropertyFlags = -1;

        static int32 FField_Next = -1;
        static int32 FField_Name = -1;
        static int32 FField_Class = -1;

        static int32 FFieldClass_CastFlags = -1;

        static int32 FProperty_Offset = -1;
        static int32 FProperty_PropertyFlags = -1;

        static int32 UFunction_ExecFunc = -1;
        static int32 UFunction_FunctionFlags = -1;

        static int32 UClass_CastFlags = -1;
    }

    namespace UnrealOptions
    {
        static bool ChunkedObjectArray = false;
        static bool FFields = false;
        static bool Doubles = false;
        static std::wstring BRMap = L"Athena_Terrain";
        static int32 PropSize = -1;
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
                // TODO There's easier ways to get this than just spamming patterns

                // 18.40
                auto Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 45 33 ED 45 8A F9").Get();

                if (!Addr) // 19.40
                    Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 4C 89 64 24 ? 55 41 55 41 57 48 8B EC 48 83 EC 50 4C 8B E9").Get();

                if (!Addr) // 4.1 to 14.60
                    Addr = Memcury::Scanner::FindStringRef(L"Illegal call to StaticFindObject() while serializing object data!").ScanFor({ 0x48, 0x89, 0x5C }, false).Get();

                CheckAddr("Failed to find StaticFindObject");

                StaticFindObject = decltype(StaticFindObject)(Addr);
            }

            // ProcessEvent
            {
                auto Addr = Memcury::Scanner::FindPattern("40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC F0 00 00 00").Get();

                if (!Addr) // 18.40
                    Addr = Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 56 41 57 48 81 EC 40 01 00 00").Get();

                if (!Addr) // 19.40
                    Addr = Memcury::Scanner::FindPattern("40 55 53 56 57 41 54 41 56 41 57 48 81 EC F0 00 00 00").Get();

                CheckAddr("Failed to find ProcessEvent");

                ProcessEventNative = decltype(ProcessEventNative)(Addr);
            }

            // Objects
            {
                // 4.1
                auto Addr = Memcury::Scanner::FindPattern("48 8B 05 ? ? ? ? 48 8D 0C 49 48 8D 14 C8 EB ? 48 8B D3 8B 42 ? C1 E8 1D A8 01 74").RelativeOffset(3).Get();

                if (!Addr) // 7.30 and 18.40  -  Can be used for 4.1 aswell if scanning for 48 8B 0D
                    Addr = Memcury::Scanner::FindStringRef(L"Material=").ScanFor({ 0x48, 0x8B, 0x05 }).RelativeOffset(3).Get();

                // if (!Addr)
                //     Addr = Memcury::Scanner::FindStringRef(L"SubmitRemoteVoiceData(%s) Size: %d received!").ScanFor({ 0x48, 0x8B, 0x05 }, true, 1).RelativeOffset(3).Get();

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

        UObject* GetOuter()
        {
            return GetChild<UObject*>(Offsets::UObject_Outer);
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

        std::string GetNameSafe()
        {
            auto ret = GetName();

            if (ret[0] >= '0' && ret[0] <= '9')
                ret[0] = '_';

            for (int i = 0; i < ret.size(); i++)
            {
                auto c = ret[i];
                if (c == '_' ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9')) continue;

                ret[i] = '_';
            }

            return ret;
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

        std::string GetPathName();
        std::string GetFullName();
        std::string GetCPPName();

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

        EPropertyFlags GetPropertyFlags()
        {
            return GetChild<EPropertyFlags>(Offsets::UProperty_PropertyFlags);
        }

        bool HasPropertyFlag(EPropertyFlags Flag)
        {
            return (GetPropertyFlags() & Flag) != CPF_None;
        }
    };

    class UEnum : public UField
    {
    };

    class UnrealProperty
    {
    public:
        template <typename T>
        T& GetChild(int32 Offset)
        {
            return *(T*)(int64(this) + Offset);
        }

        std::string GetName()
        {
            if (!this)
                return "None";

            if (UnrealOptions::FFields)
            {
                FName name = *(FName*)(int64(this) + Offsets::FField_Name);
                return name.ToString();
            }

            FName name = *(FName*)(int64(this) + Offsets::UObject_NamePrivate);
            return name.ToString();
        }

        std::string GetNameSafe()
        {
            auto ret = GetName();

            if (ret[0] >= '0' && ret[0] <= '9')
                ret[0] = '_';

            for (int i = 0; i < ret.size(); i++)
            {
                auto c = ret[i];
                if (c == '_' ||
                    (c >= 'a' && c <= 'z') ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= '0' && c <= '9')) continue;

                ret[i] = '_';
            }

            return ret;
        }

        std::string GetClassName()
        {
            if (!this)
                return "None";

            if (UnrealOptions::FFields)
            {
                // Name is at offset 0x0
                auto Class = *(void**)(int64(this) + Offsets::FField_Class);
                auto name = *(FName*)(int64(Class));
                auto id = *(uint64*)(int64(this) + 0x8);
                return std::format("{} ({}) ", name.ToString(), id);
            }

            auto Class = *(UObject**)(int64(this) + Offsets::UObject_Class);
            return Class->GetName();
        }

        std::string GetFullName()
        {
            if (!this)
                return "None";

            if (UnrealOptions::FFields)
            {
                // TODO
                return std::format("{} {}", GetClassName(), GetName());
            }

            return ((UObject*)this)->GetFullName();
        }

        EClassCastFlags GetCastFlags()
        {
            if (UnrealOptions::FFields)
            {
                auto Class = *(void**)(int64(this) + Offsets::FField_Class);
                return *(EClassCastFlags*)(int64(Class) + Offsets::FFieldClass_CastFlags);
            }

            auto Class = *(void**)(int64(this) + Offsets::UObject_Class);
            return *(EClassCastFlags*)(int64(Class) + Offsets::UClass_CastFlags);
        }

        bool HasCastFlag(EClassCastFlags Flag)
        {
            return (GetCastFlags() & Flag) != CASTCLASS_None;
        }

        UnrealProperty* GetNext()
        {
            if (UnrealOptions::FFields)
                return *(UnrealProperty**)(int64(this) + Offsets::FField_Next);

            return *(UnrealProperty**)(int64(this) + Offsets::UField_Next);
        }

        int32 GetOffset()
        {
            if (UnrealOptions::FFields)
                return *(int32*)(int64(this) + Offsets::FProperty_Offset);

            return *(int32*)(int64(this) + Offsets::UProperty_Offset);
        }

        EPropertyFlags GetPropertyFlags()
        {
            if (UnrealOptions::FFields)
                return *(EPropertyFlags*)(int64(this) + Offsets::FProperty_PropertyFlags);

            return *(EPropertyFlags*)(int64(this) + Offsets::UProperty_PropertyFlags);
        }

        bool HasPropertyFlag(EPropertyFlags Flag)
        {
            return (GetPropertyFlags() & Flag) != CPF_None;
        }

        std::string GetCPPType();
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

        UnrealProperty* GetChildProperties()
        {
            return GetChild<UnrealProperty*>(Offsets::UStruct_ChildProperties);
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

        std::vector<UFunction*> GetFuncs(bool Recursive = true)
        {
            static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");

            std::vector<UFunction*> ret;
            for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
            {
                for (auto Child = Struct->GetChildren(); Child; Child = Child->GetNext())
                {
                    if (Child->IsA(FunctionClass))
                    {
                        ret.push_back((UFunction*)Child);
                    }
                }

                if (!Recursive)
                    break;
            }
            return ret;
        }

        std::vector<UnrealProperty*> GetProps(bool Recursive = true)
        {
            std::vector<UnrealProperty*> ret;

            if (UnrealOptions::FFields)
            {
                for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
                {
                    for (auto Child = Struct->GetChildProperties(); Child; Child = Child->GetNext())
                    {
                        ret.push_back((UnrealProperty*)Child);
                    }

                    if (!Recursive)
                        break;
                }
            }
            else
            {
                static auto PropertyClass = UObject::FindClass(L"/Script/CoreUObject.Property");
                for (auto Struct = this; Struct; Struct = Struct->GetSuperStruct())
                {
                    for (auto Child = Struct->GetChildren(); Child; Child = Child->GetNext())
                    {
                        if (Child->IsA(PropertyClass))
                            ret.push_back((UnrealProperty*)Child);
                    }

                    if (!Recursive)
                        break;
                }
            }

            return ret;
        }

        int32 GetChildOffset(const std::string& Name)
        {
            for (auto Prop : GetProps())
            {
                if (Prop->GetName() == Name)
                    return Prop->GetOffset();
            }

            return -1;
        }

        UFunction* GetChildFunction(const std::string& Name);
    };

    class UClass : public UStruct
    {
        EClassCastFlags GetCastFlags()
        {
            GetChild<EClassCastFlags>(Offsets::UClass_CastFlags);
        }
    };

    class UFunction : public UStruct
    {
    public:
        void* GetExecFunc()
        {
            return GetChild<void*>(Offsets::UFunction_ExecFunc);
        }

        EFunctionFlags GetFunctionFlags()
        {
            return GetChild<EFunctionFlags>(Offsets::UFunction_FunctionFlags);
        }

        bool HasFunctionFlag(EFunctionFlags Flag)
        {
            return (GetFunctionFlags() & Flag) != EFunctionFlags::FUNC_None;
        }

        UnrealProperty* GetReturnProp()
        {
            for (auto Prop : GetProps())
            {
                if (Prop->HasPropertyFlag(CPF_ReturnParm))
                {
                    return Prop;
                }
            }

            return nullptr;
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

    std::string UObject::GetPathName()
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
        auto ret = args.ReturnValue.ToString();
        args.ReturnValue.Free();
        return ret;
    }

    std::string UObject::GetFullName()
    {
        if (!this)
            return "None";

        return std::format("{} {}", GetClass()->GetName(), GetPathName());
    }

    std::string UObject::GetCPPName()
    {
        if (!this)
            return "None";

        // TODO I probably can move this to UClass
        char Base = 'F';
        static auto ActorClass = UObject::FindClass(L"/Script/Engine.Actor");
        static auto ClassClass = UObject::FindClass(L"/Script/CoreUObject.Class");
        if (IsA(ClassClass))
        {
            auto Class = (UClass*)this;
            if (Class->IsChildOf(ActorClass)) Base = 'A';
            else Base = 'U';
        }
        return Base + GetName();
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

    UFunction* UStruct::GetChildFunction(const std::string& Name)
    {
        for (auto Func : GetFuncs())
        {
            if (Func->GetName() == Name)
                return Func;
        }

        return nullptr;
    }

    std::string UnrealProperty::GetCPPType()
    {
        if (HasCastFlag(CASTCLASS_FFloatProperty)) return "float";
        else if (HasCastFlag(CASTCLASS_FDoubleProperty)) return "double";
        else if (HasCastFlag(CASTCLASS_FInt8Property)) return "int8";
        else if (HasCastFlag(CASTCLASS_FInt16Property)) return "int16";
        else if (HasCastFlag(CASTCLASS_FIntProperty)) return "int32";
        else if (HasCastFlag(CASTCLASS_FInt64Property)) return "int64";
        else if (HasCastFlag(CASTCLASS_FUInt16Property)) return "uint16";
        else if (HasCastFlag(CASTCLASS_FUInt32Property)) return "uint32";
        else if (HasCastFlag(CASTCLASS_FUInt64Property)) return "uint64";
        else if (HasCastFlag(CASTCLASS_FNameProperty)) return "FName";
        else if (HasCastFlag(CASTCLASS_FTextProperty)) return "FText";
        else if (HasCastFlag(CASTCLASS_FStrProperty)) return "FString";
        else if (HasCastFlag(CASTCLASS_FStructProperty))
        {
            return GetChild<UStruct*>(UnrealOptions::PropSize)->GetCPPName();
        }
        else if (HasCastFlag(CASTCLASS_FBoolProperty))
        {
            auto FieldMask = GetChild<uint8>(UnrealOptions::PropSize + 3);
            bool IsNativeBool = FieldMask == 0xFF;
            if (IsNativeBool)
            {
                return "bool";
            }

            // TODO
            return "bool /*TODO*/";
        }
        else if (HasCastFlag(CASTCLASS_FArrayProperty))
        {
            return std::format("TArray<{}>", GetChild<UnrealProperty*>(UnrealOptions::PropSize)->GetCPPType());
        }
        else if (HasCastFlag(CASTCLASS_FObjectPropertyBase))
        {
            return std::format("{}*", GetChild<UClass*>(UnrealOptions::PropSize)->GetCPPName());
        }
        else if (HasCastFlag(CASTCLASS_FByteProperty))
        {
            if (auto Enum = GetChild<UEnum*>(UnrealOptions::PropSize))
            {
                // TODO
                return Enum->GetName();
            }
            return "uint8";
        }
        return "void /*TODO*/";
    }


    static void InitUnrealCore()
    {
        UObject::Init();

        // FMemory::Realloc
        {
            // 4.1
            auto Addr = Memcury::Scanner::FindPattern("4C 8B D1 48 8B 0D ? ? ? ? 48 85 C9 75 ? 49 8B CA").GetAs<void*>();

            if (!Addr)
                Addr = Memcury::Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 41 8B D8 48 8B 0D ? ? ? ? E8").GetAs<void*>();

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
            // UnrealMessageBox("{}", VerStr);
            Ver.Free();
            EngineVersion = std::stof(VerStr);
            GameVersion = std::stof(VerStr.substr(VerStr.find_last_of('-') + 1));
        }

        if (GameVersion >= 19.0f)
            UnrealOptions::BRMap = L"Artemis_Terrain";
        else if (GameVersion >= 11.0f)
            UnrealOptions::BRMap = L"Apollo_Terrain";

        UnrealOptions::ChunkedObjectArray = EngineVersion >= 4.22f; // TODO Check 4.21
        UnrealOptions::FFields = EngineVersion >= 4.25f;
        UnrealOptions::Doubles = GameVersion >= 20.0f;

        Offsets::UObject_ObjectFlags = 0x8;
        Offsets::UObject_Class = 0x10;
        Offsets::UObject_NamePrivate = 0x18;
        Offsets::UObject_Outer = 0x20;

        Offsets::UField_Next = 0x28;

        Offsets::UStruct_SuperStruct = 0x30;
        if (EngineVersion >= 4.22f) // TODO Check 4.21
            Offsets::UStruct_SuperStruct += 0x10;

        Offsets::UStruct_Children = Offsets::UStruct_SuperStruct + 0x8;
        Offsets::UStruct_Size = Offsets::UStruct_Children + 0x8;
        if (UnrealOptions::FFields)
        {
            Offsets::UStruct_ChildProperties = Offsets::UStruct_Children + 0x8;
            Offsets::UStruct_Size += 0x8;
        }
        Offsets::UStruct_Script = Offsets::UStruct_Size + 0x8;

        Offsets::UProperty_PropertyFlags = 0x38;
        Offsets::UProperty_Offset = 0x44;

        auto StructClass = UObject::FindClass(L"/Script/CoreUObject.Struct");
        auto ClassClass = UObject::FindClass(L"/Script/CoreUObject.Class");
        Offsets::UFunction_FunctionFlags = StructClass->GetSize();
        Offsets::UFunction_ExecFunc = Offsets::UFunction_FunctionFlags + 0x28;

        if (EngineVersion >= 5.0f || ClassClass->GetSize() == 0x238)
            Offsets::UClass_CastFlags = StructClass->GetSize() + 0x28;
        else if (EngineVersion >= 4.22f) // TODO Check 4.21
            Offsets::UClass_CastFlags = StructClass->GetSize() + 0x20;
        else
            Offsets::UClass_CastFlags = StructClass->GetSize() + 0x30;

        UnrealOptions::PropSize = UObject::FindClass(L"/Script/CoreUObject.Property")->GetSize();

        if (UnrealOptions::FFields)
        {
            Offsets::FField_Class = 0x8;
            Offsets::FField_Next = 0x20;
            Offsets::FField_Name = Offsets::FField_Next + 0x8;

            Offsets::FFieldClass_CastFlags = 0x10;

            Offsets::FProperty_PropertyFlags = 0x40;
            Offsets::FProperty_Offset = Offsets::FProperty_PropertyFlags + 0xC;

            UnrealOptions::PropSize = 0x78;
        }
    }
}
