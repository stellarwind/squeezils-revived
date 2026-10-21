// Mixed local/canonical input and DEF/USE/name-registry interfaces.
#include <QvDB.h>
#include <QvInput.h>
#include <QvGroup.h>
#include <QvCube.h>
#include <QvPList.h>
#include <string.h>

struct QvNodeT;
class QvInputT;
int QvReadTop(QvInputT *in, QvNodeT **out);
struct QvNameQ { void *entry; };
struct QvNodeQ;
class QvInputQ {
public:
    void Register(QvNameQ *name, QvNodeQ *node);
    QvNodeQ *Lookup(QvNameQ *name);
};
struct QvDictN {
    int tableSize; void *buckets;
    int Enter(unsigned key, void *value);
    int Find(unsigned key, void **out);
};
extern QvDictN *g_nameDict;
void QvInitNameDict();
class QvNodeS;
void QvRemoveNameS(QvNodeS *node, char *name);
void QvAddNameS(QvNodeS *node, char *name);
struct QvStrLocal {
    char *data; int capacity; char buf[32];
    QvStrLocal() { data = buf; *data = 0; }
    ~QvStrLocal();
};
class QvIn7 { public: int ReadString(QvStrLocal *text); };
typedef char InputSize[sizeof(QvInput) == 72 ? 1 : -1];

static int mixedParser()
{
    FILE *file = tmpfile(); if (!file) return 1;
    fputs("#VRML V1.0 ascii\nSeparator { DEF box Cube { width 3 height 4 depth 5 } USE box }\n", file);
    rewind(file);
    int result = 0;
    {
        QvInput input; input.setFilePointer(file);
        QvNode *root = 0;
        if (!QvReadTop((QvInputT *)&input, (QvNodeT **)&root) || !root) result = 2;
        else {
            root->ref(); QvGroup *group = (QvGroup *)root;
            if (group->getNumChildren() != 2 || group->getChild(0) != group->getChild(1)) result = 3;
            else if (((QvCube *)group->getChild(0))->width.value != 3) result = 4;
            root->unref();
        }
    }
    fclose(file); return result;
}

static int dictionaries()
{
    QvInitNameDict();
    if (!g_nameDict || g_nameDict->tableSize != 251) return 5;
    QvInput input; QvInputQ *view = (QvInputQ *)&input;
    QvName name("mixed-registry");
    QvCube *node = new QvCube; node->ref();
    view->Register((QvNameQ *)&name, (QvNodeQ *)node);
    if (view->Lookup((QvNameQ *)&name) != (QvNodeQ *)node) return 6;
    void *found = 0; unsigned key = (unsigned)name.getString();
    if (!g_nameDict->Find(key, &found)) return 7;
    QvPList *list = (QvPList *)found;
    if (list->getLength() != 1 || (*list)[0] != node) return 8;
    QvRemoveNameS((QvNodeS *)node, (char *)name.getString());
    if (list->getLength()) return 9;
    QvAddNameS((QvNodeS *)node, (char *)name.getString());
    if (list->getLength() != 1 || (*list)[0] != node) return 10;
    node->unref();
    if (list->getLength()) return 11;
    QvDict local(7); QvDictN *dict = (QvDictN *)&local;
    int a = 1, b = 2;
    if (!dict->Enter(0xffffffffU, &a) || !dict->Find(0xffffffffU, &found) || found != &a) return 12;
    if (dict->Enter(0xffffffffU, &b) || !dict->Find(0xffffffffU, &found) || found != &b) return 13;
    return 0;
}

static int mixedTokens()
{
    FILE *file = tmpfile(); if (!file) return 14;
    fputs("#VRML V1.0 ascii\n\"quoted token\" 123\n", file); rewind(file);
    int result = 0;
    {
        QvInput input; input.setFilePointer(file); QvStrLocal text;
        if (!((QvIn7 *)&input)->ReadString(&text) || strcmp(text.data, "quoted token")) result = 15;
        int number = 0; if (!result && (!input.read(number) || number != 123)) result = 16;
    }
    fclose(file); return result;
}

int main()
{
    QvDB::init();
    int result = mixedParser();
    if (!result) result = dictionaries();
    if (!result) result = mixedTokens();
    if (result) printf("Qv mixed input/name/dictionary smoke FAILED: %d\n", result);
    else puts("Qv mixed input/name/dictionary smoke passed: DEF/USE, shared registry, removal, replacement, tokens");
    return result;
}
