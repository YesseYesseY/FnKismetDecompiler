// <= UE4.23 https://github.com/EpicGames/UnrealEngine/blob/4.20/Engine/Source/Editor/UnrealEd/Public/ScriptDisassembler.h
// >= UE4.24 https://github.com/EpicGames/UnrealEngine/blob/4.24/Engine/Source/Developer/ScriptDisassembler/Public/ScriptDisassembler.h

// Base enum taken form 4.20
// Added tokens will have a comment on what UE version it was added
// Only includes changes from 4.20 to 5.4
enum EExprToken
{
    EX_LocalVariable             = 0x00,
    EX_InstanceVariable          = 0x01,
    EX_DefaultVariable           = 0x02,
    //                           = 0x03,
    EX_Return                    = 0x04,
    //                           = 0x05,
    EX_Jump                      = 0x06,
    EX_JumpIfNot                 = 0x07,
    //                           = 0x08,
    EX_Assert                    = 0x09,
    //                           = 0x0A,
    EX_Nothing                   = 0x0B,
    EX_NothingInt32              = 0x0C, // 5.3
    //                           = 0x0D,
    //                           = 0x0E,
    EX_Let                       = 0x0F,
    //                           = 0x10,
    EX_BitFieldConst             = 0x11, // 5.3
    EX_ClassContext              = 0x12,
    EX_MetaCast                  = 0x13,
    EX_LetBool                   = 0x14,
    EX_EndParmValue              = 0x15,
    EX_EndFunctionParms          = 0x16,
    EX_Self                      = 0x17,
    EX_Skip                      = 0x18,
    EX_Context                   = 0x19,
    EX_Context_FailSilent        = 0x1A,
    EX_VirtualFunction           = 0x1B,
    EX_FinalFunction             = 0x1C,
    EX_IntConst                  = 0x1D,
    EX_FloatConst                = 0x1E,
    EX_StringConst               = 0x1F,
    EX_ObjectConst               = 0x20,
    EX_NameConst                 = 0x21,
    EX_RotationConst             = 0x22,
    EX_VectorConst               = 0x23,
    EX_ByteConst                 = 0x24,
    EX_IntZero                   = 0x25,
    EX_IntOne                    = 0x26,
    EX_True                      = 0x27,
    EX_False                     = 0x28,
    EX_TextConst                 = 0x29,
    EX_NoObject                  = 0x2A,
    EX_TransformConst            = 0x2B,
    EX_IntConstByte              = 0x2C,
    EX_NoInterface               = 0x2D,
    EX_DynamicCast               = 0x2E,
    EX_StructConst               = 0x2F,
    EX_EndStructConst            = 0x30,
    EX_SetArray                  = 0x31,
    EX_EndArray                  = 0x32,
    EX_PropertyConst             = 0x33, // 4.26
    EX_UnicodeStringConst        = 0x34,
    EX_Int64Const                = 0x35,
    EX_UInt64Const               = 0x36,
    EX_DoubleConst               = 0x37, // 5.0 // There was no DoubleConst until 5.0?????????????????
    EX_PrimitiveCast             = 0x38,
    EX_SetSet                    = 0x39,
    EX_EndSet                    = 0x3A,
    EX_SetMap                    = 0x3B,
    EX_EndMap                    = 0x3C,
    EX_SetConst                  = 0x3D,
    EX_EndSetConst               = 0x3E,
    EX_MapConst                  = 0x3F,
    EX_EndMapConst               = 0x40,
    EX_Vector3fConst             = 0x41, // 5.0
    EX_StructMemberContext       = 0x42,
    EX_LetMulticastDelegate      = 0x43,
    EX_LetDelegate               = 0x44,
    EX_LocalVirtualFunction      = 0x45, // 4.22
    EX_LocalFinalFunction        = 0x46, // 4.22
    //                           = 0x47,
    EX_LocalOutVariable          = 0x48,
    //                           = 0x49,
    EX_DeprecatedOp4A            = 0x4A,
    EX_InstanceDelegate          = 0x4B,
    EX_PushExecutionFlow         = 0x4C,
    EX_PopExecutionFlow          = 0x4D,
    EX_ComputedJump              = 0x4E,
    EX_PopExecutionFlowIfNot     = 0x4F,
    EX_Breakpoint                = 0x50,
    EX_InterfaceContext          = 0x51,
    EX_ObjToInterfaceCast        = 0x52,
    EX_EndOfScript               = 0x53,
    EX_CrossInterfaceCast        = 0x54,
    EX_InterfaceToObjCast        = 0x55,
    //                           = 0x56,
    //                           = 0x57,
    //                           = 0x58,
    //                           = 0x59,
    EX_WireTracepoint            = 0x5A,
    EX_SkipOffsetConst           = 0x5B,
    EX_AddMulticastDelegate      = 0x5C,
    EX_ClearMulticastDelegate    = 0x5D,
    EX_Tracepoint                = 0x5E,
    EX_LetObj                    = 0x5F,
    EX_LetWeakObjPtr             = 0x60,
    EX_BindDelegate              = 0x61,
    EX_RemoveMulticastDelegate   = 0x62,
    EX_CallMulticastDelegate     = 0x63,
    EX_LetValueOnPersistentFrame = 0x64,
    EX_ArrayConst                = 0x65,
    EX_EndArrayConst             = 0x66,
    EX_SoftObjectConst           = 0x67,
    EX_CallMath                  = 0x68,
    EX_SwitchValue               = 0x69,
    EX_InstrumentationEvent      = 0x6A,
    EX_ArrayGetByRef             = 0x6B,
    EX_ClassSparseDataVariable   = 0x6C, // 4.24
    EX_FieldPathConst            = 0x6D, // 4.25
    //                           = 0x6E,
    //                           = 0x6F,
    EX_AutoRtfmTransact          = 0x70, // 5.3
    EX_AutoRtfmStopTransact      = 0x71, // 5.3
    EX_AutoRtfmAbortIfNot        = 0x72, // 5.3
    EX_Max                       = 0x100,
};

struct FScriptName
{
    int32 ComparisonIndex;
    int32 DisplayIndex;
    uint32 Number;

    std::string ToString()
    {
        FName name;
        name.ComparisonIndex = ComparisonIndex;
        name.Number = Number;
        return name.ToString();
    }
};

class KismetDisassembler
{
private:
    TArray<uint8> Script;
    std::string Out;
    int32 IndentCount = 0;

public:
    KismetDisassembler(UFunction* Function)
    {
        Script = Function->GetScript();
        Out = "";
    }

    template <typename T>
    T ReadBasic(int32& ScriptIndex)
    {
        T ret = *(T*)(&Script[ScriptIndex]);
        ScriptIndex += sizeof(T);
        return ret;
    }

    int32 ReadInt32(int32& ScriptIndex)
    {
        return ReadBasic<int32>(ScriptIndex);
    }

    uint64 ReadUInt64(int32& ScriptIndex)
    {
        return ReadBasic<uint64>(ScriptIndex);
    }

    template <typename T>
    T* ReadPtr(int32& ScriptIndex)
    {
        return ReadBasic<T*>(ScriptIndex);
    }

    float ReadFloat(int32& ScriptIndex)
    {
        return ReadBasic<float>(ScriptIndex);
    }

    std::string ReadName(int32& ScriptIndex)
    {
        auto ret = ((FScriptName*)(&Script[ScriptIndex]))->ToString();
        ScriptIndex += sizeof(FScriptName);
        return ret;
    }

    std::string ReadString(int32& ScriptIndex)
    {
        auto ret = std::string((const char*)(&Script[ScriptIndex]));
        ScriptIndex += ret.size() + 1;
        return ret;
    }

    void Indent()
    {
        for (int i = 0; i < IndentCount; i++)
        {
            Out += "\t";
        }
    }

    void DropIndent()
    {
        IndentCount--;
    }

    void AddIndent()
    {
        IndentCount++;
    }

#define OutLine(...) { Indent(); Out += std::format(__VA_ARGS__) + '\n'; };

    EExprToken ProcessToken(int32& ScriptIndex)
    {
        EExprToken Token = (EExprToken)Script[ScriptIndex++];
        std::println("0x{:X}", (uint8)Token);
        switch (Token)
        {
            case EX_PushExecutionFlow:
            {
                auto Skip = ReadInt32(ScriptIndex);
                OutLine("EX_PushExecutionFlow ({})", Skip);
                break;
            }
            case EX_ComputedJump:
            {
                OutLine("EX_ComputedJump");

                AddIndent();
                ProcessToken(ScriptIndex);
                DropIndent();

                break;
            }
            case EX_LocalVariable:
            {
                auto Prop = ReadPtr<UProperty>(ScriptIndex);

                OutLine("EX_LocalVariable ({})", Prop->GetFullName());

                break;
            }
            case EX_FinalFunction:
            {
                auto Func = ReadPtr<UStruct>(ScriptIndex);

                OutLine("EX_FinalFunction ({})", Func ? Func->GetFullName() : "Null");

                AddIndent();
                while (ProcessToken(ScriptIndex) != EX_EndFunctionParms) { }
                DropIndent();

                break;
            }
            case EX_EndFunctionParms:
            {
                OutLine("EX_EndFunctionParms");
                break;
            }
            case EX_LetBool:
            {
                OutLine("EX_LetBool");

                AddIndent();
                OutLine("// Var");
                ProcessToken(ScriptIndex);
                OutLine("");
                OutLine("// Exp");
                ProcessToken(ScriptIndex);
                DropIndent();

                break;
            }
            case EX_InstanceVariable:
            {
                auto Prop = ReadPtr<UProperty>(ScriptIndex);

                OutLine("EX_InstanceVariable ({})", Prop->GetFullName());
                break;
            }
            case EX_False:
            {
                OutLine("EX_False");
                break;
            }
            case EX_True:
            {
                OutLine("EX_True");
                break;
            }
            case EX_CallMulticastDelegate:
            {
                auto Func = ReadPtr<UStruct>(ScriptIndex);
                OutLine("EX_CallMulticastDelegate ({})", Func->GetFullName());

                AddIndent();
                while (ProcessToken(ScriptIndex) != EX_EndFunctionParms) {  }
                DropIndent();

                break;
            }
            case EX_VirtualFunction:
            {
                OutLine("EX_VirtualFunction ({})", ReadName(ScriptIndex));

                AddIndent();
                while (ProcessToken(ScriptIndex) != EX_EndFunctionParms) { }
                DropIndent();
                break;
            }
            case EX_PopExecutionFlow:
            {
                OutLine("EX_PopExecutionFlow");
                break;
            }
            case EX_PopExecutionFlowIfNot:
            {
                OutLine("EX_PopExecutionFlowIfNot");

                AddIndent();
                ProcessToken(ScriptIndex);
                DropIndent();
                break;
            }
            case EX_ClassContext:
            {
                OutLine("EX_ClassContext");
                goto ContextLogic;
            }
            case EX_Context:
            {
                OutLine("EX_Context");
                goto ContextLogic;
            }
            case EX_Context_FailSilent:
            {
                OutLine("EX_Context_FailSilent");
ContextLogic:
                AddIndent();
                OutLine("// Object");
                ProcessToken(ScriptIndex);
                OutLine("// Skip ({})\n", ReadInt32(ScriptIndex));

                auto Field = ReadPtr<UField>(ScriptIndex);
                OutLine("// Thing ({})\n", Field ? Field->GetFullName() : "Null");

                OutLine("// Thing2");
                ProcessToken(ScriptIndex);
                DropIndent();

                break;
            }
            case EX_ObjectConst:
            {
                auto Object = ReadPtr<UObject>(ScriptIndex);
                OutLine("EX_ObjectConst ({})", Object->GetFullName());
                break;
            }
            // case EX_StructConst:
            // {
            //     auto Struct = ReadPtr<UStruct>(ScriptIndex);
            //     auto Size = ReadInt32(ScriptIndex);
            //     OutLine("EX_StructConst ({}) (Size: {})", Struct->GetFullName(), Size);

            //     AddIndent();
            //     while (ProcessToken(ScriptIndex) != EX_EndStructConst) { }
            //     DropIndent();
            //     break;
            // }
            case EX_EndStructConst:
            {
                OutLine("EX_EndStructConst");
                break;
            }
            case EX_Self:
            {
                OutLine("EX_Self");
                break;
            }
            case EX_StringConst:
            {
                auto Str = ReadString(ScriptIndex);
                OutLine("EX_StringConst (\"{}\")", Str);
                break;
            }
            case EX_Let:
            {
                auto Prop = ReadPtr<UProperty>(ScriptIndex);
                OutLine("EX_Let ({})", Prop->GetFullName());

                AddIndent();
                OutLine("// Var");
                ProcessToken(ScriptIndex);
                OutLine("// Expr");
                ProcessToken(ScriptIndex);
                DropIndent();
                break;
            }
            case EX_CallMath:
            {
                auto Func = ReadPtr<UStruct>(ScriptIndex);
                OutLine("EX_CallMath ({})", Func->GetFullName());

                AddIndent();
                while (ProcessToken(ScriptIndex) != EX_EndFunctionParms) {  }
                DropIndent();
                break;
            }
            case EX_FloatConst:
            {
                auto Float = ReadFloat(ScriptIndex);
                OutLine("EX_FloatConst ({}f)", Float);
                break;
            }
            case EX_Jump:
            {
                auto Skip = ReadInt32(ScriptIndex);
                OutLine("EX_Jump ({})", Skip);
                break;
            }
            case EX_JumpIfNot:
            {
                auto Skip = ReadInt32(ScriptIndex);
                OutLine("EX_JumpIfNot ({})", Skip);
                AddIndent();
                ProcessToken(ScriptIndex);
                DropIndent();
                break;
            }
            case EX_Return:
            {
                OutLine("EX_Return");
                AddIndent();
                ProcessToken(ScriptIndex);
                DropIndent();
                break;
            }
            case EX_Nothing:
            {
                OutLine("EX_Nothing");
                break;
            }
            case EX_EndOfScript:
            {
                OutLine("EX_EndOfScript");
                break;
            }
            default:
            {
                OutLine("Unknown: 0x{:02X}", (uint8)Token);
                ScriptIndex = Script.Num(); // Skip it all to try avoid crashes (will prob still crash)
                break;
            }
        }

        return Token;
    }

    std::string Disassemble()
    {
        int32 ScriptIndex = 0;
        OutLine("Size: {}", Script.Num());
        while (ScriptIndex < Script.Num())
        {
            OutLine("Label_{}", ScriptIndex);
            ProcessToken(ScriptIndex);
            OutLine("");
        }
        return Out;
    }
};
