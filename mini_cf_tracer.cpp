#include "pin.H"
#include <string>

ADDRINT img_low = 0;
ADDRINT img_high = 0;

VOID instruction_analysis(ADDRINT src, std::string* ins, ADDRINT dst)
{
    printf("%016llx: %-40s -> %016llx\n", src, ins->c_str(), dst);
}

VOID instruction_instrument(INS ins, VOID* v)
{
    ADDRINT addr = INS_Address(ins);

    if (addr < img_low || addr > img_high) return;
    if (!INS_IsControlFlow(ins)) return;

    if (INS_IsValidForIpointTakenBranch(ins)) {
        std::string* str_ins_ptr = new std::string(INS_Disassemble(ins));
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(instruction_analysis), IARG_INST_PTR, IARG_PTR, str_ins_ptr, IARG_BRANCH_TARGET_ADDR, IARG_END);
    }
}

VOID image_instrument(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        img_low = IMG_LowAddress(img);
        img_high = IMG_HighAddress(img);
    }
}

int main(int argc, char* argv[])
{
    PIN_Init(argc, argv);
    IMG_AddInstrumentFunction(image_instrument, 0);
    INS_AddInstrumentFunction(instruction_instrument, 0);
    PIN_StartProgram();
    return 0;
}
