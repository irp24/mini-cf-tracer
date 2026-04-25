#include "pin.H"
#include <string>

ADDRINT img_low = 0;
ADDRINT img_high = 0;
ADDRINT data_addr = 0;
KNOB<std::string> knob_data_offset(KNOB_MODE_WRITEONCE, "pintool", "off", "0x2200", "target data offset from main module base");

VOID instruction_analysis_read(ADDRINT src, std::string* ins, ADDRINT dst)
{
    if (dst != data_addr) return;
    printf("[READ] %016llx: %-40s -> %016llx\n", src, ins->c_str(), dst);
}

VOID instruction_analysis_write(ADDRINT src, std::string* ins, ADDRINT dst)
{
    if (dst != data_addr) return;
    printf("[WRITE] %016llx: %-40s -> %016llx\n", src, ins->c_str(), dst);
}

VOID instruction_instrument(INS ins, VOID* v)
{
    ADDRINT addr = INS_Address(ins);

    if (addr < img_low || addr > img_high) return;

    UINT32 mem_operands = INS_MemoryOperandCount(ins);

    for (UINT32 mem_op = 0; mem_op < mem_operands; mem_op++)
    {
        if (INS_MemoryOperandIsRead(ins, mem_op))
        {
            std::string* str_ins_ptr = new std::string(INS_Disassemble(ins));
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)instruction_analysis_read, IARG_INST_PTR, IARG_PTR, str_ins_ptr, IARG_MEMORYOP_EA, mem_op, IARG_END);
        }

        if (INS_MemoryOperandIsWritten(ins, mem_op))
        {
            std::string* str_ins_ptr = new std::string(INS_Disassemble(ins));
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)instruction_analysis_write, IARG_INST_PTR, IARG_PTR, str_ins_ptr, IARG_MEMORYOP_EA, mem_op, IARG_END);
        }
    }
}

VOID image_instrument(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        img_low = IMG_LowAddress(img);
        img_high = IMG_HighAddress(img);
        data_addr = img_low + strtoull(knob_data_offset.Value().c_str(), nullptr, 0);
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    IMG_AddInstrumentFunction(image_instrument, 0);
    INS_AddInstrumentFunction(instruction_instrument, 0);
    PIN_StartProgram();
    return 0;
}
