// Vector3f::operator/ (0x420100). A separate unit with its own copy of the class so the frozen Vector3f unit keeps its symbol count.
struct DVec {
    float x, y, z;
    DVec() {}
    DVec(float ax, float ay, float az) : x(ax), y(ay), z(az) {}
    ~DVec() {}
    DVec operator/(float s) const;
};
DVec DVec::operator/(float s) const { return DVec(x / s, y / s, z / s); }
