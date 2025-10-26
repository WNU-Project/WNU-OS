// Windows-specific implementation for ChildProc and shell helpers.
// Kept in a separate translation unit to avoid exposing <windows.h>
// to x11.c which also includes raylib and OpenGL headers.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prevent name collisions with raylib symbols (x11.c undefines these too).
#undef Rectangle
#undef CloseWindow
#undef DrawText
#undef DrawTextEx
#undef LoadImage
#undef PlaySound
#undef ShowCursor
#undef Color
#undef Vector2

#include "shell_win.h"

#define READ_CHUNK_SIZE 512
#define OUTBUF_SIZE (READ_CHUNK_SIZE * 8)

struct ChildProc {
    HANDLE hProcess;
    HANDLE hInput;
    HANDLE hOutput;
    HANDLE hThread; // reader thread
    int running;
    CRITICAL_SECTION cs;
    char outbuf[OUTBUF_SIZE];
    int out_head;
    int out_tail;
};

static DWORD WINAPI ShellReaderThread(LPVOID arg) {
    ChildProc* p = (ChildProc*)arg;
    char tmp[READ_CHUNK_SIZE];
    DWORD got = 0;
    while (p && p->running) {
        if (p->hOutput && PeekNamedPipe(p->hOutput, NULL, 0, NULL, &got, NULL) && got > 0) {
            if (ReadFile(p->hOutput, tmp, (DWORD)sizeof(tmp), &got, NULL) && got > 0) {
                EnterCriticalSection(&p->cs);
                for (DWORD i = 0; i < got; i++) {
                    p->outbuf[p->out_head] = tmp[i];
                    p->out_head = (p->out_head + 1) % (int)sizeof(p->outbuf);
                    if (p->out_head == p->out_tail) p->out_tail = (p->out_tail + 1) % (int)sizeof(p->outbuf);
                }
                LeaveCriticalSection(&p->cs);
            }
        }
        Sleep(2);
    }
    return 0;
}

static DWORD WINAPI ShellWriteThread(LPVOID a) {
    struct { HANDLE h; char* data; int len; } *w = (void*)a;
    if (w) {
        DWORD written = 0;
        if (w->h) WriteFile(w->h, w->data, (DWORD)w->len, &written, NULL);
        if (w->data) free(w->data);
        free(w);
    }
    return 0;
}

ChildProc* CreateChildProc(void) {
    ChildProc* p = (ChildProc*)calloc(1, sizeof(ChildProc));
    if (!p) return NULL;
    InitializeCriticalSection(&p->cs);
    p->out_head = p->out_tail = 0;
    p->running = 0;
    p->hProcess = p->hInput = p->hOutput = NULL;
    p->hThread = NULL;
    return p;
}

void DestroyChildProc(ChildProc* proc) {
    if (!proc) return;
    CloseShell(proc);
    DeleteCriticalSection(&proc->cs);
    free(proc);
}

int LaunchShell(ChildProc* proc, const char* cmd) {
    if (!proc) return 0;
    HANDLE hChildStdinRd = NULL, hChildStdinWr = NULL;
    HANDLE hChildStdoutRd = NULL, hChildStdoutWr = NULL;
    SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) return 0;
    if (!SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0)) { CloseHandle(hChildStdoutRd); CloseHandle(hChildStdoutWr); return 0; }
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) { CloseHandle(hChildStdoutRd); CloseHandle(hChildStdoutWr); return 0; }
    if (!SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0)) { CloseHandle(hChildStdinRd); CloseHandle(hChildStdinWr); CloseHandle(hChildStdoutRd); CloseHandle(hChildStdoutWr); return 0; }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = hChildStdinRd;
    si.hStdOutput = hChildStdoutWr;
    si.hStdError  = hChildStdoutWr;

    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "%s", cmd);

    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
    if (!ok) {
        CloseHandle(hChildStdinWr); CloseHandle(hChildStdoutRd);
        return 0;
    }
    proc->hProcess = pi.hProcess;
    proc->hInput   = hChildStdinWr;
    proc->hOutput  = hChildStdoutRd;
    proc->running  = 1;
    proc->out_head = proc->out_tail = 0;
    CloseHandle(pi.hThread);
    DWORD tid;
    proc->hThread = CreateThread(NULL, 0, ShellReaderThread, proc, 0, &tid);
    return 1;
}

int ReadShellOutput(ChildProc* proc, char* buf, int buflen) {
    if (!proc || !buf || buflen <= 0) return 0;
    EnterCriticalSection(&proc->cs);
    int avail = (proc->out_head - proc->out_tail + (int)sizeof(proc->outbuf)) % (int)sizeof(proc->outbuf);
    if (avail == 0) { LeaveCriticalSection(&proc->cs); return 0; }
    int tocopy = (avail < buflen) ? avail : buflen;
    for (int i = 0; i < tocopy; i++) {
        buf[i] = proc->outbuf[proc->out_tail];
        proc->out_tail = (proc->out_tail + 1) % (int)sizeof(proc->outbuf);
    }
    LeaveCriticalSection(&proc->cs);
    return tocopy;
}

int WriteShellInput(ChildProc* proc, const char* buf, int buflen) {
    if (!proc || !proc->hInput || !buf || buflen <= 0) return 0;
    struct { HANDLE h; char* data; int len; } *wa = malloc(sizeof(*wa));
    if (!wa) return 0;
    wa->h = proc->hInput;
    wa->len = buflen;
    wa->data = (char*)malloc(wa->len);
    if (!wa->data) { free(wa); return 0; }
    memcpy(wa->data, buf, wa->len);
    HANDLE h = CreateThread(NULL, 0, ShellWriteThread, wa, 0, NULL);
    if (h) CloseHandle(h);
    return buflen;
}

void CloseShell(ChildProc* proc) {
    if (!proc) return;
    proc->running = 0;
    if (proc->hThread) {
        WaitForSingleObject(proc->hThread, 0);
        CloseHandle(proc->hThread); proc->hThread = NULL;
    }
    if (proc->hInput)  { CloseHandle(proc->hInput);  proc->hInput = NULL; }
    if (proc->hOutput) { CloseHandle(proc->hOutput); proc->hOutput = NULL; }
    if (proc->hProcess) {
        TerminateProcess(proc->hProcess, 0);
        CloseHandle(proc->hProcess);
        proc->hProcess = NULL;
    }
}

int ChildProcessAlive(ChildProc* proc) {
    if (!proc || !proc->hProcess) return 0;
    DWORD st = WaitForSingleObject(proc->hProcess, 0);
    return (st != WAIT_OBJECT_0);
}
