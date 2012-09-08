#include "utils.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void Spawn(char* cmd[])
{
    if(fork() == 0)
    {
        setsid();
        execvp(cmd[0], cmd);
        exit(EXIT_FAILURE);
    }
}

void CatchExitStatus(int sig)
{
    while(0 < waitpid(-1, NULL, WNOHANG));
}
