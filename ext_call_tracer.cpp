#include "pin.H"
#include <string>

ADDRINT img_low = 0;
ADDRINT img_high = 0;

VOID routine_analysis(std::string* name, ADDRINT ret)
{
    if (ret < img_low || ret > img_high) return;
    printf("%-40s is called and returns to %016llx\n", name->c_str(), ret);
}

VOID image_instrument(IMG img, VOID* v)
{
    if (IMG_IsMainExecutable(img))
    {
        img_low = IMG_LowAddress(img);
        img_high = IMG_HighAddress(img);
    }
    else
    {
        for (SYM sym = IMG_RegsymHead(img); SYM_Valid(sym); sym = SYM_Next(sym))
        {
            RTN routine = RTN_FindByAddress(IMG_LowAddress(img) + SYM_Value(sym));
            if (RTN_Valid(routine))
            {
                RTN_Open(routine);
                std::string* routine_name  = new std::string(PIN_UndecorateSymbolName(SYM_Name(sym), UNDECORATION_NAME_ONLY));
                RTN_InsertCall(routine, IPOINT_AFTER, (AFUNPTR)routine_analysis, IARG_PTR, routine_name, IARG_RETURN_IP, IARG_END);
                RTN_Close(routine);
            }

        }
    }
}

int main(int argc, char* argv[])
{
    PIN_InitSymbols();
    PIN_Init(argc, argv);
    IMG_AddInstrumentFunction(image_instrument, 0);
    PIN_StartProgram();
    return 0;
}
