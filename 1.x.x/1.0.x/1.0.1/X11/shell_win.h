// Minimal portable wrapper header for Windows-only shell helpers.
// Keeps <windows.h> and all Win32 types out of x11.c to avoid header-order
// collisions with raylib/GL headers when building with MinGW.
#ifndef SHELL_WIN_H
#define SHELL_WIN_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque shell handle
typedef struct ChildProc ChildProc;

// Allocate and free a ChildProc instance. CreateChildProc will zero the
// structure and initialize internal primitives; DestroyChildProc frees it.
ChildProc* CreateChildProc(void);
void DestroyChildProc(ChildProc* proc);

// Launch the given command in a hidden child process connected to pipes.
// Returns 1 on success, 0 on failure.
int LaunchShell(ChildProc* proc, const char* cmd);

// Read up to buflen bytes of output previously captured from the child
// process into buf. Returns number of bytes read (0 = none available).
int ReadShellOutput(ChildProc* proc, char* buf, int buflen);

// Queue a write to the child's stdin. Returns number of bytes queued (or 0).
int WriteShellInput(ChildProc* proc, const char* buf, int buflen);

// Close handles and stop background threads. Does not free the ChildProc
// structure itself; call DestroyChildProc when you are done.
void CloseShell(ChildProc* proc);

// Return non-zero if the child process is still running.
int ChildProcessAlive(ChildProc* proc);

#ifdef __cplusplus
}
#endif

#endif // SHELL_WIN_H
