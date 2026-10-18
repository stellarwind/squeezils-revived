// VC++ 4.1 integration check: parsing, field values, DEF/USE and rejection.
#include <QvDB.h>
#include <QvInput.h>
#include <QvNode.h>
#include <QvGroup.h>
#include <QvCube.h>

static int parseScene()
{
    FILE *file = tmpfile();
    if (!file) return 1;
    fputs("#VRML V1.0 ascii\nSeparator { DEF box Cube { width 3 height 4 depth 5 } USE box }\n", file);
    rewind(file);
    int result = 0;
    {
        QvInput input;
        input.setFilePointer(file);
        QvNode *root = 0;
        if (!QvDB::read(&input, root) || !root) result = 2;
        else {
            root->ref();
            QvGroup *group = (QvGroup *) root;
            if (group->getNumChildren() != 2) result = 3;
            else {
                QvCube *cube = (QvCube *) group->getChild(0);
                if (group->getChild(1) != cube) result = 4;
                if (cube->width.value != 3 || cube->height.value != 4 || cube->depth.value != 5) result = 5;
            }
            root->unref();
        }
    }
    fclose(file);
    return result;
}

static int rejectBadField()
{
    FILE *file = tmpfile();
    if (!file) return 6;
    fputs("#VRML V1.0 ascii\nCube { width nonsense }\n", file);
    rewind(file);
    int result = 0;
    {
        QvInput input;
        input.setFilePointer(file);
        QvNode *root = 0;
        if (QvDB::read(&input, root)) result = 7;
    }
    fclose(file);
    return result;
}

int main()
{
    QvDB::init();
    int result = parseScene();
    if (!result) result = rejectBadField();
    if (result) printf("QvLib smoke FAILED: %d\n", result);
    else printf("QvLib smoke passed: fields, DEF/USE, invalid input\n");
    return result;
}
