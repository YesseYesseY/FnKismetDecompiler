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
        std::println("0x{:X}", (uint8)Token);
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
                ReadPtr<UProperty>();
                break;
            }
            case EX_FinalFunction:
            {
                ReadPtr<UStruct>();
                while (PreProcessToken() != EX_EndFunctionParms) { }
                break;
            }
            case EX_EndFunctionParms:
            {
                break;
            }
            case EX_InstanceVariable:
            {
                ReadPtr<UProperty>();
                break;
            }
            case EX_False:
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
            case EX_VirtualFunction:
            {
                auto Name = ReadName();
                if (Name.starts_with("ExecuteUbergraph"))
                {
                    for (auto Child = CurrentClass->GetChildren(); Child; Child = Child->GetNext())
                    {
                        static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");
                        if (!Child->IsA(FunctionClass)) continue;
                        auto Func = (UFunction*)Child;
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

                ReadPtr<UField>();
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
                ReadPtr<UStruct>();
                ReadInt32();
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
                ReadPtr<UProperty>();
            }
            case EX_LetWeakObjPtr:
            case EX_LetObj:
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
                ReadFloat();
                ReadFloat();
                ReadFloat();
                break;
            }
            case EX_VectorConst:
            {
                ReadFloat();
                ReadFloat();
                ReadFloat();
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
            case EX_LocalOutVariable:
            {
                ReadPtr<UProperty>();
                break;
            }
            case EX_LetValueOnPersistentFrame:
            {
                ReadPtr<UProperty>()->GetFullName();
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
                ReadPtr<UProperty>();
                PreProcessToken();
                break;
            }
            case EX_ObjToInterfaceCast:
            {
                ReadPtr<UClass>();
                PreProcessToken();
                break;
            }
            case EX_ArrayConst:
            {
                ReadPtr<UProperty>();
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
                ReadPtr<UProperty>();
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

    EExprToken ProcessToken(bool CalledFromContext = false)
    {
        EExprToken Token = (EExprToken)Script[ScriptIndex++];
        std::println("0x{:X}", (uint8)Token);
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
            case EX_LocalOutVariable:
            case EX_InstanceVariable:
            case EX_LocalVariable:
            {
                auto Prop = ReadPtr<UProperty>();

                Out += Prop->GetName();

                break;
            }
            case EX_CallMath:
            case EX_CallMulticastDelegate:
            case EX_FinalFunction:
            {
                auto Func = ReadPtr<UFunction>();

                if (!CalledFromContext && Func->HasFunctionFlag(EFunctionFlags::FUNC_Static))
                    Out += std::format("{}::", Func->GetOuter()->GetCPPName());
                Out += std::format("{}(", Func->GetName());
                ArgsLoop();
                Out += ")";

                break;
            }
            case EX_EndFunctionParms:
            {
                // OutLine("EX_EndFunctionParms");
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
                auto Field = ReadPtr<UField>();

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
                auto Prop = ReadPtr<UProperty>();
                // OutLine("EX_Let ({})", Prop->GetFullName());
                goto LetLogic;
            }
            case EX_FloatConst:
            {
                Out += std::format("{:#.0f}f", ReadFloat());
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
                // OutLine("EX_JumpIfNot ({})", Skip);
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
                Out += std::format("FRotator({:#.0f}f, {:#.0f}f, {:#.0f}f)", ReadFloat(), ReadFloat(), ReadFloat());
                break;
            }
            case EX_VectorConst:
            {
                Out += std::format("FVector({:#.0f}f, {:#.0f}f, {:#.0f}f)", ReadFloat(), ReadFloat(), ReadFloat());
                break;
            }
            case EX_ByteConst:
            {
                Out += std::format("(uint8){}", ReadUInt8());
                break;
            }
            case EX_IntConst:
            {
                Out += std::format("(int32){}", ReadInt32());
                break;
            }
            case EX_LetValueOnPersistentFrame:
            {
                Out += std::format("{} = ", ReadPtr<UProperty>()->GetName());
                ProcessToken();

                break;
            }
            case EX_SwitchValue: // TODO Figure out a better way for this
            {
                auto Num = ReadUInt16();
                auto Skip = ReadInt32();
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
                break;
            }
            case EX_NoObject:
            {
                Out += "nullptr";
                break;
            }
            case EX_StructMemberContext:
            {
                auto Prop = ReadPtr<UProperty>();
                ProcessToken();
                Out += std::format(".{}", Prop->GetName());
                // OutLine("EX_StructMemberContext");

                // AddIndent();
                // OutLine("// Var ({})", ReadPtr<UProperty>()->GetFullName());
                // OutLine("");
                // OutLine("// Expr");
                // ProcessToken();
                // DropIndent();
                break;
            }
            case EX_ObjToInterfaceCast:
            {
                Out += std::format("GetInterface<I{}>(", ReadPtr<UClass>()->GetName());
                ProcessToken();
                Out += ')';
                break;
            }
            case EX_ArrayConst:
            {
                auto Prop = ReadPtr<UProperty>();
                Out += std::format("TArray<{}, {}>([", Prop->GetCPPType(), ReadInt32());
                while (ProcessToken() != EX_EndArrayConst)
                {
                    if (Script[ScriptIndex] != EX_EndArrayConst)
                    {
                        Out += ", ";
                    }
                }
                Out += "])";
                // OutLine("EX_ArrayConst (Size: {}) ({})", ReadInt32(), Prop->GetFullName());
                // AddIndent();
                // while (ProcessToken() != EX_EndArrayConst) { }
                // DropIndent();
                break;
            }
            case EX_EndArrayConst:
            {
                // OutLine("EX_EndArrayConst");
                break;
            }
            case EX_TransformConst:
            {
                Out += std::format("FTransform(FVector({:#.0f}f, {:#.0f}f, {:#.0f}f), FQuat({:#.0f}f, {:#.0f}f, {:#.0f}f, {:#.0f}f), FVector({:#.0f}f, {:#.0f}f, {:#.0f}f))",
                        ReadFloat(), ReadFloat(), ReadFloat(),
                        ReadFloat(), ReadFloat(), ReadFloat(), ReadFloat(),
                        ReadFloat(), ReadFloat(), ReadFloat());
                // OutLine("EX_TransformConst Rot ({}, {}, {}, {}), Pos ({}, {}, {}), Size ({}, {}, {})",
                //         ReadFloat(), ReadFloat(), ReadFloat(), ReadFloat(),
                //         ReadFloat(), ReadFloat(), ReadFloat(),
                //         ReadFloat(), ReadFloat(), ReadFloat()
                //         );
                break;
            }
            case EX_SkipOffsetConst:
            {
                Out += std::format("/*Skip {}*/", ReadInt32());
                break;
            }
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
                // OutLine("EX_InterfaceContext");
                // 
                // AddIndent();
                ProcessToken();
                // DropIndent();
                break;
            }
            case EX_SetArray:
            {
                OutLine("EX_SetArray");
                AddIndent();
                ProcessToken();
                AddIndent();
                while (ProcessToken() != EX_EndArray) { }
                DropIndent();
                DropIndent();
                break;
            }
            case EX_EndArray:
            {
                OutLine("EX_EndArray");
                break;
            }
            case EX_UnicodeStringConst:
            {
                OutLine("EX_UnicodeStringConst (L\"{}\")", ReadString16());
                break;
            }
            case EX_TextConst:
            {
                auto Type = (EBlueprintTextLiteralType)ReadUInt8();
                switch (Type)
                {
                    case EBlueprintTextLiteralType::Empty:
                    {
                        OutLine("EX_TextConst (Empty)");
                        break;
                    }
                    case EBlueprintTextLiteralType::LocalizedText:
                    {
                        OutLine("Ex_TextConst (Localized)");
                        AddIndent();
                        OutLine("Source = \"{}\"", ReadString());
                        OutLine("Key = \"{}\"", ReadString());
                        OutLine("NameSpace = \"{}\"", ReadString());
                        DropIndent();
                        break;
                    }
                    case EBlueprintTextLiteralType::InvariantText:
                    {
                        OutLine("Ex_TextConst (Invariant)");
                        AddIndent();
                        OutLine("Str = \"{}\"", ReadString());
                        DropIndent();
                        break;
                    }
                    case EBlueprintTextLiteralType::LiteralString:
                    {
                        OutLine("Ex_TextConst (Literal)");
                        AddIndent();
                        OutLine("Str = \"{}\"", ReadString());
                        DropIndent();
                        break;
                    }
                    case EBlueprintTextLiteralType::StringTableEntry:
                    {
                        OutLine("Ex_TextConst (StringTable)");
                        AddIndent();
                        OutLine("StringTable = \"{}\"", ReadPtr<UObject>()->GetFullName());
                        OutLine("TableId = \"{}\"", ReadString());
                        OutLine("Key = \"{}\"", ReadString());
                        DropIndent();
                        break;
                    }
                }
                
                break;
            }
            case EX_BindDelegate:
            {
                OutLine("EX_BindDelegate ({})", ReadName());

                AddIndent();
                OutLine("// Delegate");
                ProcessToken();
                OutLine("");
                OutLine("// Object");
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_AddMulticastDelegate:
            {
                OutLine("EX_AddMulticastDelegate");
                AddIndent();
                ProcessToken();
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_RemoveMulticastDelegate:
            {
                OutLine("EX_RemoveMulticastDelegate");
                AddIndent();
                ProcessToken();
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_DefaultVariable:
            {
                OutLine("EX_DefaultVariable ({})", ReadPtr<UProperty>()->GetFullName());
                break;
            }
            case EX_ClearMulticastDelegate:
            {
                OutLine("EX_ClearMulticastDelegate");
                AddIndent();
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_ArrayGetByRef:
            {
                OutLine("EX_ArrayGetByRef");
                AddIndent();
                ProcessToken();
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_MetaCast:
            {
                OutLine("EX_MetaCast ({})", ReadPtr<UClass>()->GetFullName());
                AddIndent();
                ProcessToken();
                DropIndent();
                break;
            }
            case EX_NoInterface:
            {
                OutLine("EX_NoInterface");
                break;
            }
            case EX_SetMap:
            {
                OutLine("EX_SetMap");
                AddIndent();
                ProcessToken();
                OutLine("// Size: {}", ReadInt32());
                OutLine("");
                while (ProcessToken() != EX_EndMap) { }
                DropIndent();
                break;
            }
            case EX_EndMap:
            {
                OutLine("EX_EndMap");
                break;
            }
            case EX_SoftObjectConst:
            {
                OutLine("EX_SoftObjectConst");
                AddIndent();
                ProcessToken();
                DropIndent();
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
        OutLine("void {}()", Function->GetName());
        OutLine("{{");
        AddIndent();
        ScriptIndex = 0;
        while (ScriptIndex < Script.Num())
        {
            if (std::find(Labels[Function].begin(), Labels[Function].end(), ScriptIndex) != Labels[Function].end())
            {
                Out += std::format("\nLabel_{}:\n", ScriptIndex);
            }
            Indent();
            auto Token = ProcessToken();
            // TODO Keep track of PushExecutionFlow/PopExecutionFlow to see if using return; is better than PopExecutionFlow()
            if (Token != EX_Nothing && Token != EX_EndOfScript)
                 Out += ';';
            Out += '\n';
            // OutLine("");
        }
        DropIndent();
        OutLine("}}");
        return Out;
    }

    std::string Disassemble(UClass* Class)
    {
        CurrentClass = Class;
        static auto FunctionClass = UObject::FindClass(L"/Script/CoreUObject.Function");
        for (auto Child = Class->GetChildren(); Child; Child = Child->GetNext())
        {
            if (!Child->IsA(FunctionClass)) continue;

            Script = ((UFunction*)Child)->GetScript();
            ScriptIndex = 0;
            CurrentFunc = ((UFunction*)Child);
            while (ScriptIndex < Script.Num())
            {
                PreProcessToken();
            }
        }

        for (auto Child = Class->GetChildren(); Child; Child = Child->GetNext())
        {
            if (!Child->IsA(FunctionClass)) continue;

            auto Func = (UFunction*)Child;
            Disassemble(Func);
            OutLine("");
        }

        return Out;
    }
};
