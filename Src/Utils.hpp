#include <UnrealCore.hpp>
using namespace UnrealCore;

namespace Utils
{
    void DumpObjects()
    {
        std::ofstream outfile("objects.txt");
        for (int i = 0; i < UObject::Objects->Num(); i++)
        {
            auto Object = UObject::Objects->GetObject(i);
            if (Object)
                outfile << Object->GetFullName() << '\n';
        }
        outfile.close();
    }
}
