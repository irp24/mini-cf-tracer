#include "pin.H"

ADDRINT img_low = 0;
ADDRINT img_high = 0;

VOID syscall_entry_analysis(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    ADDRINT addr = PIN_GetContextReg(ctxt, REG_INST_PTR);
    if (addr < img_low || addr > img_high) return;

    ADDRINT syscall_number = PIN_GetSyscallNumber(ctxt, std);

    printf("[syscall enter] tid=%u num=%llu\n", tid, syscall_number);

    for (UINT32 i = 0; i < 6; i++)// x86-64        rdi   rsi   rdx   r10   r8    r9    -
    {
        ADDRINT arg = PIN_GetSyscallArgument(ctxt, std, i);
        printf("arg[%u] = %llx  ", i, arg);
    }
}

VOID syscall_exit_analysis(THREADID tid, CONTEXT* ctxt, SYSCALL_STANDARD std, VOID* v)
{
    ADDRINT addr = PIN_GetContextReg(ctxt, REG_INST_PTR);
    if (addr < img_low || addr > img_high) return;

    ADDRINT ret = PIN_GetSyscallReturn(ctxt, std);

    printf("\n[syscall exit] tid=%u ret=%016llx\n\n", tid, ret);
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
    PIN_AddSyscallEntryFunction(syscall_entry_analysis, 0);
    PIN_AddSyscallExitFunction(syscall_exit_analysis, 0);
    IMG_AddInstrumentFunction(image_instrument, 0);
    PIN_StartProgram();
    return 0;
}
