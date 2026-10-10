// Vector3f's empty constructor (0x457040, `mov eax,ecx; ret`) and destructor (0x401660, `ret`),
// in their own TU so the register-frozen Vector3f unit is not perturbed.
struct Vector3fEmpty {
    float x, y, z;
    Vector3fEmpty();
    ~Vector3fEmpty();
};

Vector3fEmpty::Vector3fEmpty() {}
Vector3fEmpty::~Vector3fEmpty() {}
