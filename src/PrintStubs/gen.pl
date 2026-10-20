# generates PrintStubs.cpp and targets.txt from the shape table below
my %shape = (
 S1  => [qw(423e00 4406a0 4407f0 442380 442630)],
 S2  => [qw(4237d0 423c70 441cc0 444930 44e760)],
 S3  => [qw(423a10 4409f0 441800 444e90)],
 S5  => [qw(441a60)],
 S6  => [qw(444040)],
 S6b => [qw(421a70)],
 S7  => [qw(440060 440330 4404f0)],
 S8  => [qw(4424c0)],
 S9  => [qw(444300 444550)],
 S10 => [qw(444740)],
);
my %body = (
 S1  => 'DebugLog("%s", Name());',
 S2  => 'DebugLog("%s", Name()); a->M0(); DebugLog("\n");',
 S3  => 'DebugLog("%s", Name()); a->M0(); b->M0(); DebugLog("\n");',
 S5  => 'DebugLog("%s", Name()); a->M0(); DebugLog("%s", f8.Get());',
 S6  => 'DebugLog("%s", f8.Get());',
 S6b => 'DebugLog("%s", f12.Get());',
 S7  => 'DebugLog("%s %s", Name(), f4.Get());',
 S8  => 'DebugLog("%s %d", Name(), n4->Idx(0));',
 S9  => 'const char *s = f4.Get(); DebugLog("%s %s %d", Name(), s, n8->Idx(0)); DebugLog("\n");',
 S10 => 'DebugLog("%s", Name()); DebugLog("\n");',
);
open C, ">PrintStubs.cpp"; open T, ">targets.txt";
print C "// Print/dump methods of the script-function classes (SQZ.EXE 0x421a70-0x470400): log the class name and its operands.\n";
print C <<'H';
void DebugLog(const char *fmt, ...);   // 0x419290

struct Text { int v; const char *Get(); };     // 0x438530 (thiscall)
struct Child {
    virtual void M0(); virtual void M1(); virtual void M2(); virtual void M3(); virtual void M4();
    virtual void M5(); virtual void M6();
    virtual void M7();
};
struct Node { int Idx(int); };          // 0x421150

struct T {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual const char *Name();            // slot 4
    union { Child *a; Node *n4; Text f4; };
    union { Child *b; Node *n8; Text f8; };
    Text f12;
H
for my $s (sort keys %shape) { for my $a (@{$shape{$s}}) { print C "    void f_$a();\n"; } }
print C "};\n\n";
for my $s (sort keys %shape) { for my $a (@{$shape{$s}}) { print C "void T::f_$a()\n{\n    $body{$s}\n}\n\n"; print T "^\?f_$a\@T  $a print_$a\n"; } }
