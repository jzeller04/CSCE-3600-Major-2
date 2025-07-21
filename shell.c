// shell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 514
#define MAX_ARGS 100

void parse_command(char *line, char **args) {
    char *token = strtok(line, " \t\r\n");
    int i = 0;
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    args[i] = NULL;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    char line[MAX_LINE];

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [batchFile]\n", argv[0]);
        exit(1);
    }

    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Error opening batch file");
            exit(1);
        }
    }

    while (1) {
        if (input == stdin)
            printf("newshell> ");

        if (!fgets(line, MAX_LINE, input))
            break;

        if (strlen(line) > 512) {
            fprintf(stderr, "Input too long\n");
            continue;
        }

        // Split by ;
        char *commands[10];
        int count = 0;
        char *token = strtok(line, ";");
        while (token && count < 10) {
            commands[count++] = strdup(token);
            token = strtok(NULL, ";");
        }

        int should_exit = 0;

        for (int i = 0; i < count; i++) {
            char *args[MAX_ARGS];
            parse_command(commands[i], args);

            if (args[0] == NULL) continue;

            // === Built-in exit command ===
            if (strcmp(args[0], "exit") == 0) {
                should_exit = 1;
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) {
                // Try each directory in PATH
                char path[256];
                snprintf(path, sizeof(path), "/bin/%s", args[0]);
                execv(path, args);
                perror("Command failed");
                exit(1);
            } else {
                waitpid(pid, NULL, 0);
            }
        }

        if (should_exit)
            break;
    }

    if (input != stdin)
        fclose(input);

    return 0;
}

        
    

 
