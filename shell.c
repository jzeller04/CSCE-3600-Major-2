#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 512

// NOTE TO GROUP MEMBERS!!! My code does NOT work yet 🤣 but feel free to edit this with your parts of the project. We dont need to do the extra parts since we have only 3 ppl

int main(int argc, char* argv[])
{

    // implement exit feature to stop this probably

    char *command[100];

    if(argc > 1)
        {
            if(strcmp(argv[1], "path") == 0 && argc > 1)
            {
                // create pipe/fork
                FILE *path_pipe = popen("./path", "r");
                if(!path_pipe)
                {
                    perror("Pipe failed\n");
                    exit(EXIT_FAILURE);
                }
                // run path inside of the fork and change the path variables of the shell
                    char BUFFER[BUFFER_SIZE];
                    if(fgets(BUFFER, BUFFER_SIZE, path_pipe) != NULL)
                    {
                        // update shell path
                        if(setenv("PATH", BUFFER, 1) !=0 )
                        {
                            perror("Couldn't use setenv\n");
                            exit(EXIT_FAILURE);
                        }
                        else
                        {
                            printf("Updated shell PATH\n: %s", getenv("PATH"));
                        }
                    }
                    pclose(path_pipe);
                    
                    
                    // execlp("./path","./path", (char *)NULL);
                }
                else
                {
                    //printf("Parent\n");
                }

            }
        
    

    return 0;
}