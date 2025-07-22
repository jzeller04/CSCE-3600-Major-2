// shell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>

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
    // outer shell ignores signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    pid_t shell_pgid = getpid();
    setpgid(shell_pgid, shell_pgid); // put shell in its group
    tcsetpgrp(STDERR_FILENO, shell_pgid);



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

            // Built in PATH command
            if(strcmp(args[0], "path") == 0)
            {
                pid_t path_pid = fork();

                if (path_pid < 0)
                {
                    perror("Fork Failed\n");
                    exit(EXIT_FAILURE);
                }
                else if(path_pid == 0) // child
                {
                    setpgid(0,0);
                    tcsetpgrp(STDIN_FILENO, getpid());

                    signal(SIGINT, SIG_DFL); // set sigint back to normal for child
                    signal(SIGTSTP, SIG_DFL);
                    signal(SIGTTOU, SIG_DFL);

                    char* path_vars = getenv("PATH");
                    if(args[1] == NULL)
                    {

                        if(path_vars)
                        {
                            printf("PATH Variables:\n%s\n", path_vars);
                        }
                        else
                        {
                            printf("No PATH variables\n");
                        } 
                    } else if (strcmp(args[1], "+") == 0 && args[2] != NULL)
                    {
                        size_t length = strlen(args[2]) + strlen(path_vars) + 2; // add xtra 2 for colons
                        char* new_path_vars = malloc(length);
                        if(new_path_vars == NULL)
                        {
                            perror("malloc fail\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(new_path_vars, args[2]);
                        strcat(new_path_vars, ":"); // add colon
                        strcat(path_vars, ":");
                        strcat(path_vars, new_path_vars);

                        
                        if(setenv("PATH", path_vars, 1) != 0)
                        {
                            perror("Could not change path variables\n");
                            exit(EXIT_FAILURE);
                        }
                        printf("Added: %s to PATH variables\n", args[2]);

                        free(new_path_vars);
                    }
                    else if (strcmp(args[1], "-") == 0 && args[2] != NULL)
                    {
                        char* new_path = malloc(strlen(path_vars) + 1);
                        char* remove_var = malloc(strlen(args[2] + 1));
                        strcpy(remove_var, ":");
                        strcat(remove_var, args[2]);
                        strcat(remove_var, ":");
                        if(new_path == NULL)
                        {
                            perror("Could not copy PATH\n");
                            exit(EXIT_FAILURE);
                        }
                        strcpy(new_path, path_vars);
                        char* substring = strstr(new_path,remove_var);
                        if(substring != NULL)
                        {
                            memmove(
                                substring,
                                substring + strlen(remove_var),
                                strlen(substring + strlen(remove_var)) + 1 
                            ); // + 1 is for null character
                        }

                        if(setenv("PATH", new_path, 1) != 0)
                        {
                            perror("Could not change path variables\n");
                            exit(EXIT_FAILURE);
                        }
                        printf("Removed: %s from PATH variables\n", args[2]);

                    }
                                
                                
                }
                else
                {
                    // parent shell
                    setpgid(path_pid, path_pid);        // Set child pgid 
                    tcsetpgrp(STDIN_FILENO, path_pid);   // Give terminal to child

                    waitpid(path_pid, NULL, WUNTRACED); 

                    tcsetpgrp(STDIN_FILENO, shell_pgid); // reclaim terminal after it returns
                    // start ignoring signals again
                    signal(SIGINT, SIG_IGN);
                    signal(SIGTSTP, SIG_IGN);
                    signal(SIGTTOU, SIG_IGN);
                }
                continue;
            }


            pid_t pid = fork();
            if (pid == 0) {
                // Try each directory in PATH


                setpgid(0, 0); // new process group             
                tcsetpgrp(STDIN_FILENO, 0); 

                signal(SIGINT, SIG_DFL); // set back to default behavior for child
                signal(SIGTSTP, SIG_DFL);
                signal(SIGTTOU, SIG_DFL);
                char path[256];
                snprintf(path, sizeof(path), "/usr/bin/%s", args[0]);
                execv(path, args);
                perror("Command failed");
                //exit(127);
            } else {
                setpgid(pid, pid);        // Set child pgid 
                tcsetpgrp(STDIN_FILENO, pid);   // Give terminal to child

                int status;
                waitpid(pid, &status, WUNTRACED); 
                if (WIFSTOPPED(status)) {
                    printf("Process %d stopped\n", pid);
                }
                tcsetpgrp(STDIN_FILENO, shell_pgid); // reclaim terminal
                // start ignoring signals again
                signal(SIGINT, SIG_IGN);
                signal(SIGTSTP, SIG_IGN);
                signal(SIGTTOU, SIG_IGN);
            }
        }

        if (should_exit)
            break;
    }

    if (input != stdin)
        fclose(input);

    return 0;
}

        
    

 
