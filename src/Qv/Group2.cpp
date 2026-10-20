// QvLib group node: read all children (0x482110).
struct QvNodeG2;
class QvInputG2;
struct QvNodeListG2 { void Append(QvNodeG2 *n); };      // 0x489a10
int QvReadNode(QvInputG2 *in, QvNodeG2 **out);          // 0x47eb00
class QvGroupG2 {
public:
    int pad[2];
    QvNodeListG2 *children;                             // +8
    int ReadChildren(QvInputG2 *in);                    // 0x482110
};
int QvGroupG2::ReadChildren(QvInputG2 *in)
{
    QvNodeG2 *node;
    int ok = 1;
    if (QvReadNode(in, &node)) {
        while (node != 0) {
            children->Append(node);
            if (!QvReadNode(in, &node)) {
                ok = 0;
                break;
            }
        }
    } else
        ok = 0;
    return ok;
}
