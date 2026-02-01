// Formats TMap<>
// Example:
// TMap<int32, int32>({ 23: 45, 76: 42})
// to
// TMap<int32, int32>({
//     23: 45
//     76: 42
// })
#define FormatMaps 1

// Formats TArray<>
// Example:
// TArray<int32>({ 23, 45, 76, 42})
// to
// TArray<int32>({
//     23,
//     45,
//     76,
//     42
// })
#define FormatArrays 1

// Parses static funcs such as UKismetMathLibrary::Add_FloatFloat
// Example:
// CallFunc_Add_FloatFloat_ReturnValue_1 = UKismetMathLibrary::Add_FloatFloat(localaccumulator.SecondaryDamage_32_EB6925F04B8C5CA1EA314ABBC9C1B68F, CallFunc_SelectFloat_ReturnValue);
// to
// CallFunc_Add_FloatFloat_ReturnValue_1 = localaccumulator.SecondaryDamage_32_EB6925F04B8C5CA1EA314ABBC9C1B68F + CallFunc_SelectFloat_ReturnValue;
#define ParseStaticFuncs 1

// Prints integer types
// Example: 
// Enabled: int32(12), uint8(2)
// Disabled: 12, 2
#define TypedIntegers 1

class KismetDecompiler
{
private:
    TArray<uint8> Script;
    std::string Out;
    int32 IndentCount = 0;
    int32 ScriptIndex = 0;
    UFunction* CurrentFunc;
    UClass* CurrentClass;
    std::unordered_map<UFunction*, std::vector<int32>> Labels;

public:
    KismetDecompiler()
    {
        Out = "";
    }

    template <typename T>
    T ReadBasic()
    {
        T ret = *(T*)(&Script[ScriptIndex]);
        ScriptIndex += sizeof(T);
        return ret;
    }

    int32 ReadInt32()
    {
        return ReadBasic<int32>();
    }

    int64 ReadInt64()
    {
        return ReadBasic<int64>();
    }

    uint8 ReadUInt8()
    {
        return ReadBasic<uint8>();
    }

    uint16 ReadUInt16()
    {
        return ReadBasic<uint16>();
    }

    uint64 ReadUInt64()
    {
        return ReadBasic<uint64>();
    }

    template <typename T>
    T* ReadPtr()
    {
        return ReadBasic<T*>();
    }

    float ReadFloat()
    {
        return ReadBasic<float>();
    }

    float ReadDouble()
    {
        return ReadBasic<double>();
    }

    std::string ReadName()
    {
        auto ret = ((FScriptName*)(&Script[ScriptIndex]))->ToString();
        ScriptIndex += sizeof(FScriptName);
        return ret;
    }

    std::string ReadString8()
    {
        auto ret = std::string((const char*)(&Script[ScriptIndex]));
        ScriptIndex += ret.size() + 1;
        return ret;
    }

    std::string ReadString16()
    {
        auto retw = std::wstring((const wchar_t*)(&Script[ScriptIndex]));
        ScriptIndex += (retw.size() + 1) * 2;
        return std::string(retw.begin(), retw.end());
    }

    std::string ReadString()
    {
        auto type = (EExprToken)ReadUInt8();

        switch (type)
        {
            case EX_StringConst:
                return ReadString8();
            case EX_UnicodeStringConst:
                return ReadString16();
        }

        return "idk what happened here";
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

    void ArgsLoop(EExprToken EndToken = EX_EndFunctionParms)
    {
        while (true) {
            if (ProcessToken() == EndToken)
            {
                break;
            }
            else
            {
                if (Script[ScriptIndex] != EndToken)
                    Out += ", ";
            }
        }
    }

    EExprToken PreProcessToken()
    {
        EExprToken Token = (EExprToken)Script[ScriptIndex++];
        std::println("Pre: 0x{:X}", (uint8)Token);
        switch (Token)
        {
            case EX_PushExecutionFlow:
            {
                auto Skip = ReadInt32();
                Labels[CurrentFunc].push_back(Skip);
                break;
            }
            case EX_ComputedJump:
            {
                PreProcessToken();
                break;
            }
            case EX_LocalVariable:
            {
                ReadPtr<UnrealProperty>();
                break;
            }
            case EX_FieldPathConst:
            {
                PreProcessToken();
                break;
            }
            case EX_MapConst:
            {
                auto Key = ReadPtr<UnrealProperty>();
                auto Val = ReadPtr<UnrealProperty>();
                auto Num = ReadInt32();
                for (int i = 0; i < Num; i++)
                {
                    PreProcessToken();
                    PreProcessToken();
                }
                PreProcessToken();
                break;
            }
            case EX_EndMapConst:
            {
                break;
            }
            case EX_LocalFinalFunction:
            case EX_FinalFunction:
            {
                auto Func = ReadPtr<UFunction>();
                if (Func->GetName().starts_with("ExecuteUbergraph"))
                {
                    ScriptIndex++;
                    Labels[Func].push_back(ReadInt32());
                    ScriptIndex -= 5;
                }
                while (PreProcessToken() != EX_EndFunctionParms) { }
                break;
            }
            case EX_EndFunctionParms:
            {
                break;
            }
            case EX_InstanceVariable:
            {
                ReadPtr<UnrealProperty>();
                break;
            }
            case EX_False:
            {
                break;
            }
            case EX_IntZero:
            {
                break;
            }
            case EX_IntOne:
            {
                break;
            }
            case EX_True:
            {
                break;
            }
            case EX_CallMulticastDelegate:
            {
                ReadPtr<UStruct>();
                while (PreProcessToken() != EX_EndFunctionParms) {  }
                break;
            }
            case EX_LocalVirtualFunction:
            case EX_VirtualFunction:
            {
                auto Name = ReadName();
                if (Name.starts_with("ExecuteUbergraph"))
                {
                    for (auto Func : CurrentClass->GetFuncs())
                    {
                        if (Func->GetName().starts_with("ExecuteUbergraph"))
                        {
                            ScriptIndex++;
                            Labels[Func].push_back(ReadInt32());
                            ScriptIndex -= 5;
                            break;
                        }
                    }
                }
                while (PreProcessToken() != EX_EndFunctionParms) { }
                break;
            }
            case EX_PopExecutionFlow:
            {
                break;
            }
            case EX_PopExecutionFlowIfNot:
            {
                PreProcessToken();
                break;
            }
            case EX_ClassContext:
            case EX_Context:
            case EX_Context_FailSilent:
            {
                PreProcessToken();
                auto Skip = ReadInt32();

                ReadPtr<UnrealProperty>();
                PreProcessToken();

                break;
            }
            case EX_ObjectConst:
            {
                ReadPtr<UObject>();
                break;
            }
            case EX_StructConst:
            {
                auto Struct = ReadPtr<UStruct>();
                ReadInt32();

                static UStruct* LatentActionInfo = (UStruct*)UObject::FindObject(L"/Script/Engine.LatentActionInfo");
                if (Struct == LatentActionInfo)
                {
                    ScriptIndex++;
                    Labels[CurrentFunc].push_back(ReadInt32());
                }
                while (PreProcessToken() != EX_EndStructConst) { }
                break;
            }
            case EX_EndStructConst:
            {
                break;
            }
            case EX_Self:
            {
                break;
            }
            case EX_StringConst:
            {
                ReadString8();
                break;
            }
            case EX_Let:
            {
                ReadPtr<UnrealProperty>();
            }
            case EX_LetWeakObjPtr:
            case EX_LetObj:
            case EX_LetDelegate:
            case EX_LetBool:
            {

                PreProcessToken();
                PreProcessToken();
                break;
            }
            case EX_CallMath:
            {
                ReadPtr<UStruct>();
                while (PreProcessToken() != EX_EndFunctionParms) {  }
                break;
            }
            case EX_FloatConst:
            {
                ReadFloat();
                break;
            }
            case EX_DoubleConst:
            {
                ReadDouble();
                break;
            }
            case EX_Jump:
            {
                Labels[CurrentFunc].push_back(ReadInt32());
                break;
            }
            case EX_JumpIfNot:
            {
                Labels[CurrentFunc].push_back(ReadInt32());
                PreProcessToken();
                break;
            }
            case EX_Return:
            {
                PreProcessToken();
                break;
            }
            case EX_NothingInt32:
            case EX_Nothing:
            {
                break;
            }
            case EX_EndOfScript:
            {
                break;
            }
            case EX_NameConst:
            {
                ReadName();
                break;
            }
            case EX_RotationConst:
            {
                if (UnrealOptions::Doubles)
                {
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                }
                else
                {
                    ReadFloat();
                    ReadFloat();
                    ReadFloat();
                }
                break;
            }
            case EX_VectorConst:
            {
                if (UnrealOptions::Doubles)
                {
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                }
                else
                {
                    ReadFloat();
                    ReadFloat();
                    ReadFloat();
                }
                break;
            }
            case EX_ByteConst:
            {
                ReadUInt8();
                break;
            }
            case EX_IntConst:
            {
                ReadInt32();
                break;
            }
            case EX_Int64Const:
            {
                ReadInt64();
                break;
            }
            case EX_UInt64Const:
            {
                ReadUInt64();
                break;
            }
            case EX_LocalOutVariable:
            {
                ReadPtr<UnrealProperty>();
                break;
            }
            case EX_LetValueOnPersistentFrame:
            {
                ReadPtr<UnrealProperty>()->GetFullName();
                PreProcessToken();
                break;
            }
            case EX_SwitchValue:
            {
                auto Num = ReadUInt16();
                auto Skip = ReadInt32();
                PreProcessToken();
                for (uint16 i = 0; i < Num; i++)
                {
                    PreProcessToken();
                    auto Next = ReadInt32();
                    PreProcessToken();
                }
                PreProcessToken();
                break;
            }
            case EX_NoObject:
            {
                break;
            }
            case EX_StructMemberContext:
            {
                ReadPtr<UnrealProperty>();
                PreProcessToken();
                break;
            }
            case EX_InterfaceToObjCast:
            case EX_ObjToInterfaceCast:
            {
                ReadPtr<UClass>();
                PreProcessToken();
                break;
            }
            case EX_ArrayConst:
            {
                ReadPtr<UnrealProperty>();
                ReadInt32();
                while (PreProcessToken() != EX_EndArrayConst) { }
                break;
            }
            case EX_EndArrayConst:
            {
                break;
            }
            case EX_TransformConst:
            {
                if (UnrealOptions::Doubles)
                {
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                    ReadDouble();
                }
                else
                {
                    ReadFloat();
                    ReadFloat(); 
                    ReadFloat();
                    ReadFloat();
                    ReadFloat();
                    ReadFloat(); 
                    ReadFloat();
                    ReadFloat();
                    ReadFloat();
                    ReadFloat();
                }
                break;
            }
            case EX_SkipOffsetConst:
            {
                auto Skip = ReadInt32();
                break;
            }
            case EX_DynamicCast:
            {
                ReadPtr<UClass>();
                PreProcessToken();
                break;
            }
            case EX_PrimitiveCast:
            {
                ReadUInt8();
                PreProcessToken();
                break;
            }
            case EX_InterfaceContext:
            {
                PreProcessToken();
                break;
            }
            case EX_SetArray:
            {
                PreProcessToken();
                while (PreProcessToken() != EX_EndArray) { }
                break;
            }
            case EX_EndArray:
            {
                break;
            }
            case EX_UnicodeStringConst:
            {
                ReadString16();
                break;
            }
            case EX_TextConst:
            {
                auto Type = (EBlueprintTextLiteralType)ReadUInt8();
                switch (Type)
                {
                    case EBlueprintTextLiteralType::Empty:
                    {
                        break;
                    }
                    case EBlueprintTextLiteralType::LocalizedText:
                    {
                        ReadString();
                        ReadString();
                        ReadString();
                        break;
                    }
                    case EBlueprintTextLiteralType::InvariantText:
                    {
                        ReadString();
                        break;
                    }
                    case EBlueprintTextLiteralType::LiteralString:
                    {
                        ReadString();
                        break;
                    }
                    case EBlueprintTextLiteralType::StringTableEntry:
                    {
                        ReadPtr<UObject>();
                        ReadString();
                        ReadString();
                        break;
                    }
                }
                
                break;
            }
            case EX_BindDelegate:
            {
                ReadName();

                PreProcessToken();
                PreProcessToken();
                break;
            }
            case EX_AddMulticastDelegate:
            {
                PreProcessToken();
                PreProcessToken();
                break;
            }
            case EX_RemoveMulticastDelegate:
            {
                PreProcessToken();
                PreProcessToken();
                break;
            }
            case EX_DefaultVariable:
            {
                ReadPtr<UnrealProperty>();
                break;
            }
            case EX_ClearMulticastDelegate:
            {
                PreProcessToken();
                break;
            }
            case EX_ArrayGetByRef:
            {
                PreProcessToken();
                PreProcessToken();
                break;
            }
            case EX_MetaCast:
            {
                ReadPtr<UClass>();
                PreProcessToken();
                break;
            }
            case EX_NoInterface:
            {
                break;
            }
            case EX_SetMap:
            {
                PreProcessToken();
                ReadInt32();
                while (PreProcessToken() != EX_EndMap) { }
                break;
            }
            case EX_EndMap:
            {
                break;
            }
            case EX_SoftObjectConst:
            {
                PreProcessToken();
                break;
            }
            default:
            {
                break;
            }
        }

        return Token;
    }

    // Implementing this for the 4th time !!!
    template <typename T>
    T& BaseGetChild(void* Base, int32 Offset)
    {
        return *(T*)(int64(Base) + Offset);
    }

    void ProcessDefault(UnrealProperty* Prop, void* Base, int32 Offset)
    {
        if (Prop->HasCastFlag(CASTCLASS_FFloatProperty)) Out += std::format("{:#.0f}f", BaseGetChild<float>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FBoolProperty)) Out += std::format("{}", BaseGetChild<bool>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FInt8Property)) Out += std::format("{}", BaseGetChild<int8>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FInt16Property)) Out += std::format("{}", BaseGetChild<int16>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FIntProperty)) Out += std::format("{}", BaseGetChild<int32>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FInt64Property)) Out += std::format("{}", BaseGetChild<int64>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FUInt16Property)) Out += std::format("{}", BaseGetChild<uint16>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FUInt32Property)) Out += std::format("{}", BaseGetChild<uint32>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FUInt64Property)) Out += std::format("{}", BaseGetChild<uint64>(Base, Offset));
        else if (Prop->HasCastFlag(CASTCLASS_FNameProperty)) Out += std::format("FName(\"{}\")", BaseGetChild<FName>(Base, Offset).ToString());
        else if (Prop->HasCastFlag(CASTCLASS_FStrProperty)) Out += std::format("FString(\"{}\")", BaseGetChild<FString>(Base, Offset).ToString());
        else if (Prop->HasCastFlag(CASTCLASS_FObjectPropertyBase) && !Prop->HasCastFlag(CASTCLASS_FSoftObjectProperty)) // TODO SoftObject
        {
            auto Obj = BaseGetChild<UObject*>(Base, Offset);
            if (Obj)
                Out += std::format("UObject::FindObject(\"{}\")", Obj->GetPathName());
            else
                Out += "nullptr";
        }
        else if (Prop->HasCastFlag(CASTCLASS_FStructProperty))
        {
            auto Struct = (UStruct*)Prop->GetChild<UStruct*>(UnrealOptions::PropSize);
            Out += std::format("{}({{\n", Struct->GetCPPName());
            AddIndent();
            auto Props = Struct->GetProps();
            for (int i = 0; i < Props.size(); i++)
            {
                Indent();
                Out += std::format("{} = ", Props[i]->GetNameSafe());
                ProcessDefault(Props[i], Base, Offset + Props[i]->GetOffset());
                if (i != Props.size() - 1)
                    Out += ',';
                Out += '\n';
            }
            DropIndent();
            Indent();
            Out += "})";
        }
        else if (Prop->HasCastFlag(CASTCLASS_FArrayProperty))
        {
            auto Arr = BaseGetChild<TArray<uint8>>(Base, Offset);
            auto ArrProp = Prop->GetChild<UnrealProperty*>(UnrealOptions::PropSize);
            if (Arr.Num() == 0)
            {
                Out += std::format("TArray<{}>()", ArrProp->GetCPPType());
                return;
            }
            Out += std::format("TArray<{}>({{ ", ArrProp->GetCPPType());

#if FormatArrays
            Out += '\n';
            AddIndent();
            Indent();
#endif
            for (int i = 0; i < Arr.Num(); i++)
            {
                ProcessDefault(ArrProp, Arr.GetData(), i * ArrProp->GetSize());
                if (i != Arr.Num() - 1)
                {
                    Out += ", ";
#if FormatArrays
                    Out += "\n";
                    Indent();
#endif
                }
            }
#if FormatArrays
            Out += "\n";
            DropIndent();
            Indent();
#endif
            Out += "})";
            // Out += std::format(" /* {} */", ArrProp->GetCPPType());
            // ret = std::format("TArray<{}>", GetChild<UnrealProperty*>(UnrealOptions::PropSize)->GetCPPType());
        }
        else if (Prop->HasCastFlag(CASTCLASS_FByteProperty))
        {
            // TODO

            Out += std::format("{}", BaseGetChild<uint8>(Base, Offset));
        }
        else
        {
            Out += "{/*TODO*/}";
        }
    }

    void ProcessDefault(UnrealProperty* Prop)
    {
        auto Default = CurrentClass->GetDefaultObject();
        if (!Default)
            return;

        Out += " = ";
        ProcessDefault(Prop, Default, Prop->GetOffset());
    }

    void ProcessMap()
    {
        Out += '{';
#if FormatMaps
        AddIndent();
#else
        Out += ' ';
#endif
        auto Num = ReadInt32();
        for (int i = 0; i < Num; i++)
        {
#if FormatMaps
            Out += '\n';
            Indent();
#endif
            ProcessToken();
            Out += ": ";
            ProcessToken();
            if (i != Num - 1)
                Out += ", ";
        }
#if FormatMaps
        Out += '\n';
        DropIndent();
        Indent();
#else
        Out += ' ';
#endif
        Out += '}';

        ProcessToken();
    }

    EExprToken ProcessToken(bool CalledFromContext = false)
    {
        EExprToken Token = (EExprToken)Script[ScriptIndex++];
        std::println("Main: 0x{:X}", (uint8)Token);
        switch (Token)
        {
            case EX_PushExecutionFlow:
            {
                Out += std::format("PushExecutionFlow({})", ReadInt32());
                break;
            }
            case EX_ComputedJump:
            {
                Out += "ComputedJump(";
                ProcessToken();
                Out += ')';

                break;
            }
            case EX_DefaultVariable:
            case EX_LocalOutVariable:
            case EX_InstanceVariable:
            case EX_LocalVariable:
            {
                auto Prop = ReadPtr<UnrealProperty>();

                if (Prop->HasPropertyFlag(CPF_OutParm))
                    Out += '*';
                Out += Prop->GetNameSafe();

                break;
            }
            case EX_CallMath:
            case EX_CallMulticastDelegate:
            case EX_LocalFinalFunction:
            case EX_FinalFunction:
            {
                auto Func = ReadPtr<UFunction>();

                bool StaticParsed = false;
#if ParseStaticFuncs
                // UKismetSystemLibrary::IsValid
                // UKismetMathLibrary::Conv_*

#define BasicStaticMathOp(Operation) { StaticParsed = true; ProcessToken(); Out += " " Operation " "; ProcessToken(); }

                static auto MathLibClass = UObject::FindClass(L"/Script/Engine.KismetMathLibrary");
                auto FuncName = Func->GetNameSafe();
                if (Func->GetOuter() == MathLibClass)
                {
                    if (FuncName.starts_with("Add_")) BasicStaticMathOp("+")
                    else if (FuncName.starts_with("Multiply_")) BasicStaticMathOp("*")
                    else if (FuncName.starts_with("Subtract_")) BasicStaticMathOp("-")
                    else if (FuncName.starts_with("Divide_")) BasicStaticMathOp("/")
                    else if (FuncName.starts_with("Less_")) BasicStaticMathOp("<")
                    else if (FuncName.starts_with("LessEqual_")) BasicStaticMathOp("<=")
                    else if (FuncName.starts_with("Greater_")) BasicStaticMathOp(">")
                    else if (FuncName.starts_with("GreaterEqual_")) BasicStaticMathOp(">=")
                    else if (FuncName.starts_with("EqualEqual_")) BasicStaticMathOp("==")
                    else if (FuncName.starts_with("NotEqual_")) BasicStaticMathOp("!=")
                    else if (FuncName == "BooleanAND") BasicStaticMathOp("&&")
                    else if (FuncName == "BooleanOR") BasicStaticMathOp("||")
                    else if (FuncName == "Not_PreBool")
                    {
                        StaticParsed = true;
                        Out += "!";
                        ProcessToken();
                    }
                    else if (FuncName.starts_with("Select"))
                    {
                        StaticParsed = true;
                        int32 CurrIndex = ScriptIndex;
                        PreProcessToken();
                        PreProcessToken();
                        ProcessToken();
                        Out += " ? ";
                        ScriptIndex = CurrIndex;
                        ProcessToken();
                        Out += " : ";
                        ProcessToken();
                        PreProcessToken();
                    }
                    else if (FuncName == "MakeRotator")
                    {
                        StaticParsed = true;
                        Out += "FRotator(";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ")";
                    }
                    else if (FuncName == "MakeVector")
                    {
                        StaticParsed = true;
                        Out += "FVector(";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ")";
                    }
                    else if (FuncName == "MakeTransform")
                    {
                        StaticParsed = true;
                        Out += "FTransform(";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ", ";
                        ProcessToken();
                        Out += ")";
                    }
                }
#endif

                if (!StaticParsed)
                {
                    if (!CalledFromContext && Func->HasFunctionFlag(EFunctionFlags::FUNC_Static))
                        Out += std::format("{}::", Func->GetOuter()->GetCPPName());
                    Out += std::format("{}(", Func->GetNameSafe());
                    ArgsLoop();
                    Out += ")";
                }
                else
                {
                    ProcessToken(); // EX_EndFunctionParms
                }

                break;
            }
            case EX_EndFunctionParms:
            {
                break;
            }
            case EX_False:
            {
                Out += "false";
                break;
            }
            case EX_True:
            {
                Out += "true";
                break;
            }
            case EX_LocalVirtualFunction:
            case EX_VirtualFunction:
            {
                Out += std::format("{}(", ReadName());
                ArgsLoop();
                Out += ")";

                break;
            }
            case EX_PopExecutionFlow:
            {
                Out += "PopExecutionFlow()";
                break;
            }
            case EX_PopExecutionFlowIfNot:
            {
                Out += "if (!";
                ProcessToken();
                Out += ") PopExecutionFlow()";
                break;
            }
            case EX_ClassContext:
            case EX_Context:
            case EX_Context_FailSilent: // TODO? Mark as failsilent?
            {
                ProcessToken();
                Out += "->";

                auto Skip = ReadInt32();
                auto Field = ReadPtr<UnrealProperty>();

                ProcessToken(true);

                break;
            }
            case EX_ObjectConst:
            {
                auto Object = ReadPtr<UObject>();
                Out += std::format("UObject::FindObject(\"{}\")", Object->GetPathName()); // TODO? Add class as template?
                break;
            }
            case EX_StructConst:
            {
                auto Struct = ReadPtr<UStruct>();
                auto Size = ReadInt32();
                Out += std::format("{}(", Struct->GetCPPName());
                ArgsLoop(EX_EndStructConst);
                Out += ')';
                break;
            }
            case EX_EndStructConst:
            {
                break;
            }
            case EX_Self:
            {
                Out += "this";
                break;
            }
            case EX_StringConst:
            {
                Out += std::format("FString(\"{}\")", ReadString8());
                break;
            }
            case EX_LetWeakObjPtr:
            case EX_LetObj:
            case EX_LetDelegate:
            case EX_LetBool:
            {
LetLogic:
                ProcessToken();
                Out += " = ";
                ProcessToken();
                break;
            }
            case EX_Let:
            {
                auto Prop = ReadPtr<UnrealProperty>();
                goto LetLogic;
            }
            case EX_FloatConst:
            {
                Out += std::format("{:#.0f}f", ReadFloat());
                break;
            }
            case EX_DoubleConst:
            {
                Out += std::format("{:#.0f}d", ReadDouble());
                break;
            }
            case EX_Jump:
            {
                Out += std::format("goto Label_{}", ReadInt32());
                break;
            }
            case EX_JumpIfNot:
            {
                auto Skip = ReadInt32();
                Out += "if (!";
                ProcessToken();
                Out += std::format(") goto Label_{}", Skip);
                break;
            }
            case EX_Return:
            {
                Out += "return";
                if (Script[ScriptIndex] != EX_Nothing)
                    Out += ' ';
                ProcessToken();
                break;
            }
            case EX_NothingInt32:
            case EX_Nothing:
            {
                // OutLine("EX_Nothing");
                break;
            }
            case EX_EndOfScript:
            {
                // OutLine("EX_EndOfScript");
                break;
            }
            case EX_NameConst:
            {
                Out += std::format("FName(\"{}\")", ReadName());
                break;
            }
            case EX_RotationConst:
            {
                if (UnrealOptions::Doubles)
                    Out += std::format("FRotator({:#.0f}d, {:#.0f}d, {:#.0f}d)", ReadDouble(), ReadDouble(), ReadDouble());
                else
                    Out += std::format("FRotator({:#.0f}f, {:#.0f}f, {:#.0f}f)", ReadFloat(), ReadFloat(), ReadFloat());
                break;
            }
            case EX_VectorConst:
            {
                if (UnrealOptions::Doubles)
                    Out += std::format("FVector({:#.0f}d, {:#.0f}d, {:#.0f}d)", ReadDouble(), ReadDouble(), ReadDouble());
                else
                    Out += std::format("FVector({:#.0f}f, {:#.0f}f, {:#.0f}f)", ReadFloat(), ReadFloat(), ReadFloat());
                break;
            }
            case EX_ByteConst:
            {
#if TypedIntegers
                Out += std::format("uint8({})", ReadUInt8());
#else
                Out += std::format("{}", ReadUInt8());
#endif
                break;
            }
            case EX_IntConst:
            {
#if TypedIntegers
                Out += std::format("int32({})", ReadInt32());
#else
                Out += std::format("{}", ReadInt32());
#endif
                break;
            }
            case EX_Int64Const:
            {
#if TypedIntegers
                Out += std::format("int64({})", ReadInt64());
#else
                Out += std::format("{}", ReadInt64());
#endif
                break;
            }
            case EX_UInt64Const:
            {
#if TypedIntegers
                Out += std::format("uint64({})", ReadUInt64());
#else
                Out += std::format("{}", ReadUInt64());
#endif
                break;
            }
            case EX_LetValueOnPersistentFrame:
            {
                Out += std::format("{} = ", ReadPtr<UnrealProperty>()->GetNameSafe());
                ProcessToken();

                break;
            }
            case EX_SwitchValue: // TODO Figure out a better way for this
            {
                auto Num = ReadUInt16();
                auto Skip = ReadInt32();

                UnrealProperty* Prop = nullptr;
                if (
                    Script[ScriptIndex] == EX_DefaultVariable
                    || Script[ScriptIndex] == EX_LocalOutVariable
                    || Script[ScriptIndex] == EX_InstanceVariable
                    || Script[ScriptIndex] == EX_LocalVariable
                    )
                {
                    auto Idx = ScriptIndex;
                    ScriptIndex++;
                    Prop = ReadPtr<UnrealProperty>();
                    ScriptIndex = Idx;
                }

                if (Num == 2 && Prop->HasCastFlag(CASTCLASS_FBoolProperty))
                {
                    ProcessToken();
                    Out += " ? ";
                    if (Script[ScriptIndex] == EX_True)
                    {
                        ScriptIndex++;
                        ReadInt32();
                        ProcessToken();
                        Out += " : ";
                        ScriptIndex++;
                        ReadInt32();
                        ProcessToken();
                    }
                    else
                    {
                        ScriptIndex++;
                        ReadInt32();
                        auto Idx = ScriptIndex;
                        PreProcessToken();
                        ScriptIndex++;
                        ReadInt32();
                        ProcessToken();
                        Out += " : ";
                        auto Idx2 = ScriptIndex;
                        ScriptIndex = Idx;
                        ProcessToken();
                        ScriptIndex = Idx2;
                    }

                    PreProcessToken();
                }
                else
                {
                    Out += "switch (";
                    ProcessToken();
                    Out += ") { ";
                    for (uint16 i = 0; i < Num; i++)
                    {
                        Out += "case ";
                        ProcessToken();
                        Out += ": ";
                        ReadInt32();
                        ProcessToken();
                        Out += "; ";
                    }
                    Out += "default: ";
                    ProcessToken();
                    Out += "; }";
                }
                break;
            }
            case EX_NoInterface:
            case EX_NoObject:
            {
                Out += "nullptr";
                break;
            }
            case EX_StructMemberContext:
            {
                auto Prop = ReadPtr<UnrealProperty>();
                ProcessToken();
                Out += std::format(".{}", Prop->GetNameSafe());
                break;
            }
            case EX_ObjToInterfaceCast:
            {
                Out += std::format("GetInterface<I{}>(", ReadPtr<UClass>()->GetNameSafe());
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_InterfaceToObjCast:
            {
                Out += std::format("GetObject<{}>(", ReadPtr<UClass>()->GetCPPName());
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_ArrayConst:
            {
                auto Prop = ReadPtr<UnrealProperty>();
                Out += std::format("TArray<{}, {}>([", Prop->GetCPPType(), ReadInt32());
                while (ProcessToken() != EX_EndArrayConst)
                {
                    if (Script[ScriptIndex] != EX_EndArrayConst)
                    {
                        Out += ", ";
                    }
                }
                Out += "])";
                break;
            }
            case EX_EndArrayConst:
            {
                break;
            }
            case EX_TransformConst:
            {
                if (UnrealOptions::Doubles)
                {
                    Out += std::format("FTransform(FVector({:#.0f}d, {:#.0f}d, {:#.0f}d), FQuat({:#.0f}d, {:#.0f}d, {:#.0f}d, {:#.0f}d), FVector({:#.0f}d, {:#.0f}d, {:#.0f}d))", ReadDouble(), ReadDouble(), ReadDouble(),
                          ReadDouble(), ReadDouble(), ReadDouble(), ReadDouble(),
                          ReadDouble(), ReadDouble(), ReadDouble());
                }
                else
                {
                    Out += std::format("FTransform(FVector({:#.0f}f, {:#.0f}f, {:#.0f}f), FQuat({:#.0f}f, {:#.0f}f, {:#.0f}f, {:#.0f}f), FVector({:#.0f}f, {:#.0f}f, {:#.0f}f))", ReadFloat(), ReadFloat(), ReadFloat(),
                          ReadFloat(), ReadFloat(), ReadFloat(), ReadFloat(),
                          ReadFloat(), ReadFloat(), ReadFloat());
                }
                break;
            }
            case EX_SkipOffsetConst:
            {
#if TypedIntegers
                Out += std::format("int32({})", ReadInt32());
#else
                Out += std::format("{}", ReadInt32());
#endif
                break;
            }
            case EX_IntZero:
            {
                Out += '0';
                break;
            }
            case EX_IntOne:
            {
                Out += '1';
                break;
            }
            case EX_MetaCast:
            case EX_DynamicCast:
            {
                Out += std::format("Cast<{}>(", ReadPtr<UClass>()->GetCPPName());
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_PrimitiveCast: // TODO
            {
                Out += std::format("PrimitiveCast({}, ", ReadUInt8());
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_InterfaceContext:
            {
                ProcessToken();
                break;
            }
            case EX_SetArray:
            {
                ProcessToken();
                Out += " = [";
                while (ProcessToken() != EX_EndArray)
                {
                    if (Script[ScriptIndex] != EX_EndArray)
                    {
                        Out += ", ";
                    }
                }
                Out += ']';
                break;
            }
            case EX_EndArray:
            {
                break;
            }
            case EX_UnicodeStringConst:
            {
                Out += std::format("L\"{}\"", ReadString16());
                break;
            }
            case EX_TextConst:
            {
                auto Type = (EBlueprintTextLiteralType)ReadUInt8();
                switch (Type)
                {
                    case EBlueprintTextLiteralType::Empty:
                    {
                        Out += "FText::GetEmpty()"; // FText() also works
                        break;
                    }
                    case EBlueprintTextLiteralType::LocalizedText:
                    {
                        Out += std::format("FText::Localized(\"{}\", \"{}\", \"{}\")", ReadString(), ReadString(), ReadString());
                        break;
                    }
                    case EBlueprintTextLiteralType::InvariantText:
                    {
                        Out += std::format("FText::Invariant(\"{}\")", ReadString());
                        break;
                    }
                    case EBlueprintTextLiteralType::LiteralString:
                    {
                        Out += std::format("FText::Literal(\"{}\")", ReadString());
                        break;
                    }
                    case EBlueprintTextLiteralType::StringTableEntry:
                    {
                        // TODO I haven't found a single function that uses this so i have no idea what to expect
                        ReadPtr<UObject>();
                        ReadString();
                        ReadString();
                        break;
                    }
                }
                
                break;
            }
            case EX_BindDelegate:
            {
                auto Event = ReadName();
                ProcessToken();
                Out += std::format(".Bind(\"{}\", ", Event);
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_AddMulticastDelegate:
            {
                ProcessToken();
                Out += ".Add(";
                ProcessToken();
                Out += ")";
                break;
            }
            case EX_RemoveMulticastDelegate:
            {
                ProcessToken();
                Out += ".Remove(";
                ProcessToken();
                Out += ")";
                break;
            }
            case EX_ClearMulticastDelegate:
            {
                ProcessToken();
                Out += ".Clear()";
                break;
            }
            case EX_ArrayGetByRef:
            {
                ProcessToken();
                Out += '[';
                ProcessToken();
                Out += ']';
                break;
            }

            /* 
               Wild that in all of 8.51 there is only 1 blueprint that uses EX_SetMap (atleast when loaded into Athena_Terrain) 
               [yes@arch decomp]$ grep -r "EX_"
               MinigameSettingsMachine_C.cpp:				EX_SetMap
               MinigameSettingsMachine_C.cpp:				EX_SetMap
            */
            case EX_SetMap:
            {
                ProcessToken();
                Out += ".Set(";
                ProcessMap();
                Out += ')';
                break;
            }
            case EX_EndMap:
            {
                break;
            }
            case EX_SoftObjectConst:
            {
                Out += "TSoftObjectPtr(";
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_FieldPathConst:
            {
                Out += "TFieldPath(";
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_MapConst:
            {
                auto Key = ReadPtr<UnrealProperty>();
                auto Val = ReadPtr<UnrealProperty>();
                Out += std::format("TMap<{}, {}>(", Key->GetCPPType(), Val->GetCPPType());
                ProcessMap();
                Out += ')';
                break;
            }
            case EX_EndMapConst:
            {
                break;
            }
            default:
            {
#ifdef SEARCH_FOR_UNKNOWNS
                MessageBox("UNKNOWN AT {}", CurrentFunc->GetFullName());
#endif
                OutLine("Unknown: 0x{:02X}", (uint8)Token);
                ScriptIndex = Script.Num(); // Skip it all to try avoid crashes (will prob still crash)
                break;
            }
        }

        return Token;
    }

    std::string Disassemble(UFunction* Function)
    {
        CurrentFunc = Function;
        Script = Function->GetScript();
        OutLine("// Script Size: {}", Script.Num());
        std::string rettype = "void";
        if (auto retprop = Function->GetReturnProp())
            rettype = retprop->GetCPPType();

        Indent();
        Out += std::format("{} {}(", rettype, Function->GetNameSafe());
        std::vector<UnrealProperty*> Parms;
        for (auto Prop : Function->GetProps())
        {
            if (Prop->HasPropertyFlag(CPF_Parm))
            {
                Parms.push_back(Prop);
            }
        }
        for (int i = 0; i < Parms.size(); i++)
        {
            Out += std::format("{}{} {}", Parms[i]->GetCPPType(), Parms[i]->HasPropertyFlag(CPF_OutParm) ? "*" : "", Parms[i]->GetNameSafe());
            if (i != Parms.size() - 1)
                Out += ", ";
        }
        Out += ")\n";
        OutLine("{{");
        AddIndent();
        ScriptIndex = 0;
        while (ScriptIndex < Script.Num())
        {
            if (std::find(Labels[Function].begin(), Labels[Function].end(), ScriptIndex) != Labels[Function].end())
            {
                Out += std::format("\nLabel_{}:\n", ScriptIndex);
            }
            if (Script[ScriptIndex] != EX_EndOfScript)
                Indent();

            auto Token = ProcessToken();
            if (Token != EX_Nothing && Token != EX_EndOfScript)
                 Out += ';';
            if (Token != EX_EndOfScript)
                Out += '\n';
        }
        DropIndent();
        OutLine("}}");
        return Out;
    }

    std::string Disassemble(UClass* Class)
    {
        CurrentClass = Class;
        static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");
        for (auto Func : Class->GetFuncs(false))
        {
            Script = Func->GetScript();
            ScriptIndex = 0;
            CurrentFunc = Func;
            while (ScriptIndex < Script.Num())
            {
                PreProcessToken();
            }
        }

        Out += std::format("class {} : public {}\n{{\n", Class->GetCPPName(), Class->GetSuperStruct()->GetCPPName());
        AddIndent();
        for (auto Prop : Class->GetProps(false))
        {
            Indent();
            Out += std::format("{} {}", Prop->GetCPPType(), Prop->GetNameSafe());
            ProcessDefault(Prop);
            Out += ";\n";
        }
        OutLine("");
        for (auto Func : Class->GetFuncs(false))
        {
            Disassemble(Func);
            OutLine("");
        }
        DropIndent();
        Out += "}";

        return Out;
    }
};
