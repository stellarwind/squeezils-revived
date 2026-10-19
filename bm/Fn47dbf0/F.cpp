// PaceMat::Transpose (0x47dbf0): copies the 4x4 matrix and writes it back transposed; returns this.
struct Mat {
    float m[16];
    Mat() {}
    ~Mat();                                              // empty, out of line (0x402310)
    Mat(const Mat &o)
    {
        m[0] = o.m[0]; m[1] = o.m[1]; m[2] = o.m[2]; m[3] = o.m[3];
        m[4] = o.m[4]; m[5] = o.m[5]; m[6] = o.m[6]; m[7] = o.m[7];
        m[8] = o.m[8]; m[9] = o.m[9]; m[10] = o.m[10]; m[11] = o.m[11];
        m[12] = o.m[12]; m[13] = o.m[13]; m[14] = o.m[14]; m[15] = o.m[15];
    }
    Mat &Transpose();
};
Mat &Mat::Transpose()
{
    Mat t(*this);
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m[i * 4 + j] = t.m[j * 4 + i];
    return *this;
}
