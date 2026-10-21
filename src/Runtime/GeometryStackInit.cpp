// Original initializer 0x424dd0; separate entry following StackTop (0x424dc0).
void Thunk424de0(); // 0x424de0, Thunks/Thunks.cpp: construct stack at 0x4c71c0.
void R424df0();     // 0x424df0, Guards/Init.cpp: register G424e00 with atexit.

void InitializeGeometryStack()
{
    Thunk424de0();
    R424df0();
}
