#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    // first part done

    if(argc == 1)
    {
        char* path_vars = getenv("PATH");

        if(path_vars)
        {
            printf("PATH Variables:\n%s\n", path_vars);
        }
        else
        {
            printf("No PATH variables\n");
        }
    }
    else if(argc == 3)
    {
        if(strcmp(argv[1], "+") == 0)
        {
            
            setenv("PATH", argv[2], 1);
        }
    }


}