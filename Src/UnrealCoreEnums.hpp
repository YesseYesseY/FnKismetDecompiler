#include <vector>

#define ENUM_CLASS_FLAGS(Enum) \
    inline constexpr Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
    inline constexpr Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
    inline constexpr Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
    inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
    inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
    inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
    inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
    inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }

namespace UnrealCore
{
    enum class EObjectFlags : int32
    {
        RF_NoFlags                      = 0x00000000,
        RF_Public                       = 0x00000001,
        RF_Standalone                   = 0x00000002,
        RF_MarkAsNative                 = 0x00000004,
        RF_Transactional                = 0x00000008,
        RF_ClassDefaultObject           = 0x00000010,
        RF_ArchetypeObject              = 0x00000020,
        RF_Transient                    = 0x00000040,
        RF_MarkAsRootSet                = 0x00000080,
        RF_TagGarbageTemp               = 0x00000100,
        RF_NeedInitialization           = 0x00000200,
        RF_NeedLoad                     = 0x00000400,
        RF_NeedPostLoad                 = 0x00001000,
        RF_NeedPostLoadSubobjects       = 0x00002000,
        RF_NewerVersionExists           = 0x00004000,
        RF_BeginDestroyed               = 0x00008000,
        RF_FinishDestroyed              = 0x00010000,
        RF_BeingRegenerated             = 0x00020000,
        RF_DefaultSubObject             = 0x00040000,
        RF_WasLoaded                    = 0x00080000,
        RF_TextExportTransient          = 0x00100000,
        RF_LoadCompleted                = 0x00200000,
        RF_InheritableComponentTemplate = 0x00400000,
        RF_DuplicateTransient           = 0x00800000,
        RF_StrongRefOnFrame             = 0x01000000,
        RF_NonPIEDuplicateTransient     = 0x02000000,
        RF_ImmutableDefaultObject       = 0x04000000,
        RF_WillBeLoaded                 = 0x08000000,
        RF_HasExternalPackage           = 0x10000000,
        RF_MigratingAsset               = 0x20000000,
        RF_MirroredGarbage              = 0x40000000,
        RF_AllocatedInSharedPage        = 0x80000000,
    };

    enum EFunctionFlags : uint32
    {
        FUNC_None                = 0x00000000,
        FUNC_Final                = 0x00000001,
        FUNC_RequiredAPI            = 0x00000002,
        FUNC_BlueprintAuthorityOnly= 0x00000004,
        FUNC_BlueprintCosmetic    = 0x00000008,
        FUNC_Net                = 0x00000040,
        FUNC_NetReliable        = 0x00000080,
        FUNC_NetRequest            = 0x00000100,
        FUNC_Exec                = 0x00000200,
        FUNC_Native                = 0x00000400,
        FUNC_Event                = 0x00000800,
        FUNC_NetResponse        = 0x00001000,
        FUNC_Static                = 0x00002000,
        FUNC_NetMulticast        = 0x00004000,
        FUNC_UbergraphFunction    = 0x00008000,
        FUNC_MulticastDelegate    = 0x00010000,
        FUNC_Public                = 0x00020000,
        FUNC_Private            = 0x00040000,
        FUNC_Protected            = 0x00080000,
        FUNC_Delegate            = 0x00100000,
        FUNC_NetServer            = 0x00200000,
        FUNC_HasOutParms        = 0x00400000,
        FUNC_HasDefaults        = 0x00800000,
        FUNC_NetClient            = 0x01000000,
        FUNC_DLLImport            = 0x02000000,
        FUNC_BlueprintCallable    = 0x04000000,
        FUNC_BlueprintEvent        = 0x08000000,
        FUNC_BlueprintPure        = 0x10000000,
        FUNC_EditorOnly            = 0x20000000,
        FUNC_Const                = 0x40000000,
        FUNC_NetValidate        = 0x80000000,
    
        FUNC_AllFlags        = 0xFFFFFFFF,
    };

    ENUM_CLASS_FLAGS(EObjectFlags);
    ENUM_CLASS_FLAGS(EFunctionFlags);
}
