# generates MFSetNum.cpp: SetNum(int) of the multi-valued fields (value array at +0x14, count at +0xc and +0x10)
my @v = ([ '48a0e0', 8, 'MFA' ], [ '48ab90', 12, 'MFB' ], [ '48aff0', 12, 'MFC' ], [ '48b250', 4, 'MFD' ], [ '48b480', 4, 'MFE' ]);
open O, ">MFSetNum.cpp";
print O "// SetNum of the multi-valued QvLib fields (value array at +0x14, element size 4/8/12, malloc-or-new switch per class).\n#include <stdlib.h>\n";
for my $x (@v) { my ($a, $sz, $c) = @$x;
 print O "extern int g_use$c;\nclass Qv$c { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };\n";
 print O "void Qv$c\::SetNum(int n)\n{\n    int *old = vals;\n    if (old == 0) {\n        if (n > 0) {\n            if (g_use$c)\n                vals = (int *)malloc(n * $sz);\n            else\n                vals = (int *)operator new(n * $sz);\n        }\n    } else if (n > 0) {\n        if (g_use$c)\n            vals = (int *)realloc(vals, n * $sz);\n        else {\n            vals = (int *)operator new(n * $sz);\n            for (int i = 0; count > i; i++) {\n                if (i >= n)\n                    break;\n                vals[i] = old[i];\n            }\n            delete old;\n        }\n    } else {\n        if (g_use$c)\n            free(old);\n        else\n            delete old;\n        vals = 0;\n    }\n    count2 = n;\n    count = n;\n}\n\n"; }
close O;
open T, ">targets_mfset.txt"; print T "#FLAGS -GX -GR -Gy -O2 -G3 -ML\n"; for my $x (@v) { print T '^\?SetNum@Qv' . $x->[2] . '  ' . $x->[0] . ' mfset_' . $x->[0] . "\n" } close T;
