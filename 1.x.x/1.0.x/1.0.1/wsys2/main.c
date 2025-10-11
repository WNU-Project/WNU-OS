#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <direct.h>
#include "wsys2.h"

void print_banner() {
    printf("\033[36m╔══════════════════════════════════════╗\033[0m\n");
    printf("\033[36m║              WSYS2 v1.0.0            ║\033[0m\n");
    printf("\033[36m║         WNU Package Manager          ║\033[0m\n");
    printf("\033[36m╚══════════════════════════════════════╝\033[0m\n");
    printf("\n");
}

void print_help() {
    printf("Usage: wsys2 <command> [options] [package]\n\n");
    printf("Commands:\n");
    printf("  \033[32minstall\033[0m <package>    Install a package from .wnupkg file\n");
    printf("  \033[31mremove\033[0m  <package>    Remove an installed package\n");
    printf("  \033[33mupdate\033[0m              Update all installed packages\n");
    printf("  \033[34msearch\033[0m  <term>      Search for available packages\n");
    printf("  \033[35mlist\033[0m                List all installed packages\n");
    printf("  \033[36minfo\033[0m    <package>    Show package information\n");
    printf("  \033[95monline\033[0m  <command>    Online package operations\n");
    printf("  \033[37mversion\033[0m             Show WSYS2 version\n");
    printf("  \033[37mhelp\033[0m                Show this help message\n");
    printf("\nOnline Commands:\n");
    printf("  \033[95monline update\033[0m       Refresh online package database\n");
    printf("  \033[95monline search\033[0m <term> Search online packages\n");
    printf("  \033[95monline install\033[0m <pkg> Install from online repository\n");
    printf("  \033[95monline list\033[0m         List all available online packages\n");
    printf("  \033[95monline info\033[0m <pkg>   Show online package information\n");
    printf("\nExamples:\n");
    printf("  wsys2 install vim.wnupkg\n");
    printf("  wsys2 remove vim\n");
    printf("  wsys2 list\n");
    printf("  wsys2 search editor\n");
    printf("  wsys2 online update\n");
    printf("  wsys2 online install git\n");
    printf("  wsys2 online search dev\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_banner();
        print_help();
        return 1;
    }

    // Initialize WSYS2 system
    if (wsys2_init() != 0) {
        printf("\033[31mError:\033[0m Failed to initialize WSYS2 system\n");
        return 1;
    }

    const char* command = argv[1];

    if (strcmp(command, "install") == 0) {
        if (argc < 3) {
            printf("\033[31mError:\033[0m No package specified for installation\n");
            printf("Usage: wsys2 install <package.wnupkg>\n");
            return 1;
        }
        return wsys2_install(argv[2]);
    }
    else if (strcmp(command, "remove") == 0 || strcmp(command, "uninstall") == 0) {
        if (argc < 3) {
            printf("\033[31mError:\033[0m No package specified for removal\n");
            printf("Usage: wsys2 remove <package>\n");
            return 1;
        }
        return wsys2_remove(argv[2]);
    }
    else if (strcmp(command, "update") == 0) {
        return wsys2_update();
    }
    else if (strcmp(command, "search") == 0) {
        if (argc < 3) {
            return wsys2_search(NULL); // Show all available packages
        }
        return wsys2_search(argv[2]);
    }
    else if (strcmp(command, "list") == 0) {
        return wsys2_list();
    }
    else if (strcmp(command, "info") == 0) {
        if (argc < 3) {
            printf("\033[31mError:\033[0m No package specified for info\n");
            printf("Usage: wsys2 info <package>\n");
            return 1;
        }
        return wsys2_info(argv[2]);
    }
    else if (strcmp(command, "online") == 0) {
        if (argc < 3) {
            printf("\033[31mError:\033[0m No online command specified\n");
            printf("Usage: wsys2 online <update|search|install|list|info> [args]\n");
            return 1;
        }
        const char* online_cmd = argv[2];
        
        if (strcmp(online_cmd, "update") == 0) {
            return wsys2_online_update();
        }
        else if (strcmp(online_cmd, "search") == 0) {
            if (argc < 4) {
                return wsys2_online_search(NULL); // Show all
            }
            return wsys2_online_search(argv[3]);
        }
        else if (strcmp(online_cmd, "install") == 0) {
            if (argc < 4) {
                printf("\033[31mError:\033[0m No package specified for online installation\n");
                printf("Usage: wsys2 online install <package>\n");
                return 1;
            }
            return wsys2_online_install(argv[3]);
        }
        else if (strcmp(online_cmd, "list") == 0) {
            return wsys2_online_list();
        }
        else if (strcmp(online_cmd, "info") == 0) {
            if (argc < 4) {
                printf("\033[31mError:\033[0m No package specified for online info\n");
                printf("Usage: wsys2 online info <package>\n");
                return 1;
            }
            return wsys2_online_info(argv[3]);
        }
        else {
            printf("\033[31mError:\033[0m Unknown online command '%s'\n", online_cmd);
            printf("Available commands: update, search, install, list, info\n");
            return 1;
        }
    }
    else if (strcmp(command, "version") == 0 || strcmp(command, "--version") == 0) {
        printf("WSYS2 Package Manager v1.0.0\n");
        printf("Part of WNU OS 1.0.1\n");
        printf("Package format: .wnupkg\n");
        return 0;
    }
    else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0) {
        print_banner();
        print_help();
        return 0;
    }
    else if (strcmp(command, "wsys2") == 0) {
        while (1) {
            char input[256];
            printf("wsys2> ");
            if (!fgets(input, sizeof(input), stdin)) {
                break; // EOF or error
            }
            // Remove newline
            input[strcspn(input, "\n")] = 0;

            // Tokenize input
            char* args[10];
            int arg_count = 0;
            char* token = strtok(input, " ");
            while (token && arg_count < 10) {
                args[arg_count++] = token;
                token = strtok(NULL, " ");
            }

            if (arg_count == 0) continue; // Empty input

            // Handle commands
            if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0) {
                break;
            } else if (strcmp(args[0], "install") == 0) {
                if (arg_count < 2) {
                    printf("\033[31mError:\033[0m No package specified for installation\n");
                    continue;
                }
                wsys2_install(args[1]);
            } else if (strcmp(args[0], "remove") == 0 || strcmp(args[0], "uninstall") == 0) {
                if (arg_count < 2) {
                    printf("\033[31mError:\033[0m No package specified for removal\n");
                    continue;
                }
                wsys2_remove(args[1]);
            } else if (strcmp(args[0], "update") == 0) {
                wsys2_update();
            } else if (strcmp(args[0], "search") == 0) {
                if (arg_count < 2) {
                    wsys2_search(NULL); // Show all available packages
                } else {
                    wsys2_search(args[1]);
                }
            } else if (strcmp(args[0], "list") == 0) {
                wsys2_list();
            } else if (strcmp(args[0], "info") == 0) {
                if (arg_count < 2) {
                    printf("\033[31mError:\033[0m No package specified for info\n");
                    continue;
                }
                wsys2_info(args[1]);
            } else if (strcmp(args[0], "version") == 0 || strcmp(args[0], "--version") == 0) {
                printf("WSYS2 Package Manager v1.0.0\n");
                printf("Part of WNU OS 1.0.1\n");
                printf("Package format: .wnupkg\n");
            } else if (strcmp(args[0], "online") == 0) {
                if (arg_count < 2) {
                    printf("\033[31mError:\033[0m No online command specified\n");
                    printf("Usage: online <update|search|install|list|info> [args]\n");
                    continue;
                }
                
                if (strcmp(args[1], "update") == 0) {
                    wsys2_online_update();
                } else if (strcmp(args[1], "search") == 0) {
                    if (arg_count < 3) {
                        wsys2_online_search(NULL); // Show all
                    } else {
                        wsys2_online_search(args[2]);
                    }
                } else if (strcmp(args[1], "install") == 0) {
                    if (arg_count < 3) {
                        printf("\033[31mError:\033[0m No package specified for online installation\n");
                        printf("Usage: online install <package>\n");
                        continue;
                    }
                    wsys2_online_install(args[2]);
                } else if (strcmp(args[1], "list") == 0) {
                    wsys2_online_list();
                } else if (strcmp(args[1], "info") == 0) {
                    if (arg_count < 3) {
                        printf("\033[31mError:\033[0m No package specified for online info\n");
                        printf("Usage: online info <package>\n");
                        continue;
                    }
                    wsys2_online_info(args[2]);
                } else {
                    printf("\033[31mError:\033[0m Unknown online command '%s'\n", args[1]);
                    printf("Available commands: update, search, install, list, info\n");
                }
            } else if (strcmp(args[0], "help") == 0 || strcmp(args[0], "--help") == 0) {
                print_help();
            } else {
                printf("\033[31mError:\033[0m Unknown command '%s'\n", args[0]);
                printf("Type 'help' for available commands\n");
            }
        }
        return 0;
    }
    else {
        printf("\033[31mError:\033[0m Unknown command '%s'\n", command);
        printf("Run 'wsys2 help' for available commands\n");
        return 1;
    }

    return 0;
}
