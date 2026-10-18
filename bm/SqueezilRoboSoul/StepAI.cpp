// SqueezilRoboSoul::StepAI (SQZ.EXE 0x404a80): ask the questions in turn until one gives a state.
struct RoboAI {
    char pad[0x80];
    int step;                       // 0x80
    int PickState();                // 0x404840
    int SenseState(int kind);       // 0x404920
    int StepAI();
};

int RoboAI::StepAI()
{
    int result;
    do {
        step++;
        switch (step - 1) {
        case 0: result = PickState(); break;
        case 1: result = SenseState(0); break;
        case 2: result = SenseState(1); break;
        case 3: result = SenseState(2); break;
        case 4: step = 0; result = 0x13; break;
        }
    } while (result == 0x13 && step != 0);
    return result;
}
