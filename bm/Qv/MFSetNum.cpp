// SetNum of the multi-valued QvLib fields (value array at +0x14, element size 4/8/12, malloc-or-new switch per class).
#include <stdlib.h>
extern int g_useMFA;
class QvMFA { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };
void QvMFA::SetNum(int n)
{
    int *old = vals;
    if (old == 0) {
        if (n > 0) {
            if (g_useMFA)
                vals = (int *)malloc(n * 8);
            else
                vals = (int *)operator new(n * 8);
        }
    } else if (n > 0) {
        if (g_useMFA)
            vals = (int *)realloc(vals, n * 8);
        else {
            vals = (int *)operator new(n * 8);
            for (int i = 0; count > i; i++) {
                if (i >= n)
                    break;
                vals[i] = old[i];
            }
            delete old;
        }
    } else {
        if (g_useMFA)
            free(old);
        else
            delete old;
        vals = 0;
    }
    count2 = n;
    count = n;
}

extern int g_useMFB;
class QvMFB { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };
void QvMFB::SetNum(int n)
{
    int *old = vals;
    if (old == 0) {
        if (n > 0) {
            if (g_useMFB)
                vals = (int *)malloc(n * 12);
            else
                vals = (int *)operator new(n * 12);
        }
    } else if (n > 0) {
        if (g_useMFB)
            vals = (int *)realloc(vals, n * 12);
        else {
            vals = (int *)operator new(n * 12);
            for (int i = 0; count > i; i++) {
                if (i >= n)
                    break;
                vals[i] = old[i];
            }
            delete old;
        }
    } else {
        if (g_useMFB)
            free(old);
        else
            delete old;
        vals = 0;
    }
    count2 = n;
    count = n;
}

extern int g_useMFC;
class QvMFC { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };
void QvMFC::SetNum(int n)
{
    int *old = vals;
    if (old == 0) {
        if (n > 0) {
            if (g_useMFC)
                vals = (int *)malloc(n * 12);
            else
                vals = (int *)operator new(n * 12);
        }
    } else if (n > 0) {
        if (g_useMFC)
            vals = (int *)realloc(vals, n * 12);
        else {
            vals = (int *)operator new(n * 12);
            for (int i = 0; count > i; i++) {
                if (i >= n)
                    break;
                vals[i] = old[i];
            }
            delete old;
        }
    } else {
        if (g_useMFC)
            free(old);
        else
            delete old;
        vals = 0;
    }
    count2 = n;
    count = n;
}

extern int g_useMFD;
class QvMFD { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };
void QvMFD::SetNum(int n)
{
    int *old = vals;
    if (old == 0) {
        if (n > 0) {
            if (g_useMFD)
                vals = (int *)malloc(n * 4);
            else
                vals = (int *)operator new(n * 4);
        }
    } else if (n > 0) {
        if (g_useMFD)
            vals = (int *)realloc(vals, n * 4);
        else {
            vals = (int *)operator new(n * 4);
            for (int i = 0; count > i; i++) {
                if (i >= n)
                    break;
                vals[i] = old[i];
            }
            delete old;
        }
    } else {
        if (g_useMFD)
            free(old);
        else
            delete old;
        vals = 0;
    }
    count2 = n;
    count = n;
}

extern int g_useMFE;
class QvMFE { public: int p0, p4, p8, count, count2; int *vals; void SetNum(int n); };
void QvMFE::SetNum(int n)
{
    int *old = vals;
    if (old == 0) {
        if (n > 0) {
            if (g_useMFE)
                vals = (int *)malloc(n * 4);
            else
                vals = (int *)operator new(n * 4);
        }
    } else if (n > 0) {
        if (g_useMFE)
            vals = (int *)realloc(vals, n * 4);
        else {
            vals = (int *)operator new(n * 4);
            for (int i = 0; count > i; i++) {
                if (i >= n)
                    break;
                vals[i] = old[i];
            }
            delete old;
        }
    } else {
        if (g_useMFE)
            free(old);
        else
            delete old;
        vals = 0;
    }
    count2 = n;
    count = n;
}

