#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{

    // implement exit feature to stop this probably

    char *command[100];

    if(argc > 1)
        {
            if(strcmp(argv[1], "path") == 0)
            {
                __pid_t pid = fork();
                if(pid < 0)
                {
                    perror("Fork Not Succesful\n");
                }
                else if(pid == 0)
                {
                    // forked
                    execlp("./path","./path", (char *)NULL);
                }
                else
                {
                    //printf("Parent\n");
                }

            }
        }

    

    return 0;
}