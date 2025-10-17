#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stddef.h>

// Prototypes (inlined to avoid include permission issues during build)
int tty_session_save(const char* tty, const char* username, const char* cwd);
int tty_session_load(const char* tty, char* username_out, size_t user_sz, char* cwd_out, size_t cwd_sz);

// Session filename placed in the top-level 1.0.1 directory: "tty_<tty>.session"
// e.g., tty_tty1.session

static void build_session_path(const char* tty, char* out, size_t out_sz) {
    snprintf(out, out_sz, "tty_%s.session", tty);
}

int tty_session_save(const char* tty, const char* username, const char* cwd) {
    // Ensure sessions directory exists
    CreateDirectoryA("C:\\WNU\\WNU OS\\sessions", NULL);
    if (!tty || !cwd) return 1;
    if (!username || strlen(username) == 0) username = "root"; // fallback

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "C:\\WNU\\WNU OS\\sessions\\%s.session", tty);

    FILE* f = fopen(path, "w");
    if (!f) return 2;

    fprintf(f, "username=%s\n", username);
    fprintf(f, "cwd=%s\n", cwd);
    fclose(f);
    return 0;
}


int tty_session_load(const char* tty, char* username_out, size_t user_sz, char* cwd_out, size_t cwd_sz) {
    if (!tty || !username_out || !cwd_out) return 1;

    char path[MAX_PATH];
    snprintf(path, sizeof(path), "C:\\WNU\\WNU OS\\sessions\\%s.session", tty);

    FILE* f = fopen(path, "r");
    if (!f) return 2;

    char line[1024];
    username_out[0] = '\0';
    cwd_out[0] = '\0';

    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "username=", 9) == 0) {
            strncpy(username_out, line + 9, user_sz - 1);
            username_out[user_sz - 1] = '\0';
        } else if (strncmp(line, "cwd=", 4) == 0) {
            strncpy(cwd_out, line + 4, cwd_sz - 1);
            cwd_out[cwd_sz - 1] = '\0';
        }
    }
    fclose(f);

    // Set defaults if missing
    if (username_out[0] == '\0') strncpy(username_out, "root", user_sz - 1);
    if (cwd_out[0] == '\0') strncpy(cwd_out, "C:\\", cwd_sz - 1);

    return 0;
}


// Optional CLI for manual testing. Define TTY_SESSION_CLI to enable this main()
#ifdef TTY_SESSION_CLI
int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s save <tty> <username> <cwd> | load <tty>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "save") == 0) {
        if (argc < 5) {
            printf("Usage: %s save <tty> <username> <cwd>\n", argv[0]);
            return 1;
        }
        int r = tty_session_save(argv[2], argv[3], argv[4]);
        if (r == 0) printf("Saved session for %s\n", argv[2]);
        else printf("Failed to save session (code %d)\n", r);
        return r;
    } else if (strcmp(argv[1], "load") == 0) {
        char user[256];
        char cwd[1024];
        int r = tty_session_load(argv[2], user, sizeof(user), cwd, sizeof(cwd));
        if (r == 0) {
            printf("tty: %s\n", argv[2]);
            printf("username=%s\n", user);
            printf("cwd=%s\n", cwd);
            return 0;
        } else {
            printf("Failed to load session (code %d)\n", r);
            return r;
        }
    }

    printf("Unknown command\n");
    return 1;
}
#endif /* TTY_SESSION_CLI */
