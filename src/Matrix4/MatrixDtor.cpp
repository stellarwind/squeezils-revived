// Original matrix complete destructor, 0x402310: ret. No fields are destroyed.
// SIZE 0x40
struct MatrixLifetime {
    float elements[16]; // 0x00
    ~MatrixLifetime();
};

MatrixLifetime::~MatrixLifetime() {}
