#ifndef TTY_SESSION_H
#define TTY_SESSION_H

#include <stddef.h>

// Save the session information for 'tty' (e.g., "tty1").
// Returns 0 on success, non-zero on failure.
int tty_session_save(const char* tty, const char* username, const char* cwd);

// Load the session information for 'tty'.
// username_out and cwd_out must be buffers of user_sz and cwd_sz respectively.
// Returns 0 on success, non-zero on failure or if file not present.
int tty_session_load(const char* tty, char* username_out, size_t user_sz, char* cwd_out, size_t cwd_sz);

// Utility CLI: if built as an executable, calling with:
//   tty_session save <tty> <username> <cwd>
//   tty_session load <tty>

#endif // TTY_SESSION_H
