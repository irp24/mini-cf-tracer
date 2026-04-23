#include "pin.H"
#include <cstdio>
#include <string>

ADDRINT imgLow = 0;
ADDRINT imgHigh = 0;

VOID LogEdge(ADDRINT src, std::string* ins, ADDRINT dst)
{
    printf("%016llx: %-40s -> %016llx\n", (unsigned long long)src, ins->c_str(), (unsigned long long)dst);
}

VOID InstrumentInstruction(INS ins, VOID* v)
{
    ADDRINT addr = INS_Address(ins);

    if (addr < imgLow || addr > imgHigh) return;
    if (!INS_IsControlFlow(ins)) return;

    if (INS_IsValidForIpointTakenBranch(ins)) {
        std::string* str_ins_ptr = new std::string(INS_Disassemble(ins));
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(LogEdge), IARG_INST_PTR, IARG_PTR, str_ins_ptr, IARG_BRANCH_TARGET_ADDR, IARG_END);
    }
}

VOID ImageLoad(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        imgLow = IMG_LowAddress(img);
        imgHigh = IMG_HighAddress(img);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);

    IMG_AddInstrumentFunction(ImageLoad, 0);
    INS_AddInstrumentFunction(InstrumentInstruction, 0);

    PIN_StartProgram();
    return 0;
}
