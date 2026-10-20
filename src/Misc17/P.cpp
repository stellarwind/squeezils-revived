// SQZ.EXE 0x479af0: reports a texture load failure through the VRML error printer, returns 0
void Log(const char *fmt, const char *name);       // 0x47a7b0 (cdecl)
int F479af0(const char *name) {
    Log("Can't load %s\n", name);
    return 0;
}
