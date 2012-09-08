#include "utils.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void Spawn(char* command[])
{
    if(fork() == 0)
    {
        setsid();
        execvp(command[0], command);
        exit(EXIT_FAILURE);
    }
}

void CatchExitStatus(int signum)
{
    while(0 < waitpid(-1, NULL, WNOHANG));
}
