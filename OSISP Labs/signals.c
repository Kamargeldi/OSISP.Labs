#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

int pid[3];
int temp;

void parent_handler(int signum, siginfo_t *si, void *ucontext)
{
    if (signum == SIGUSR1)
    {
        union sigval sv = si->si_value;
        printf("Parent: sent count of byte to process 2\n");
        sigqueue(pid[1], SIGUSR1, sv);
        sleep(5);
        kill(0, SIGINT);
    }
}

void proc1_handler(int signum, siginfo_t *si, void *ucontext)
{
    if (signum == SIGUSR1)
    {
        union sigval value = si->si_value;
        int byte_count = strlen(value.sival_ptr);
        value.sival_int = byte_count;
        printf("Process 1: sent count of byte to parent\n");
        sigqueue(getppid(), SIGUSR1, value);
        exit(0);
    }
}


void proc2_handler(int signum, siginfo_t *si, void *ucontext)
{
    if (signum == SIGUSR1)
    {
        union sigval sv = si->si_value;
        for (int i = 0; i < sv.sival_int; i++)
        {
            if (temp == getpid())
            {
                int child = fork();
                if (child == 0)
                {
                    pause();
                    exit(0);
                }
            }
        }
        if (temp == getpid())
        {
           printf("Process 2: created %d child processes\n", sv.sival_int);
        }
    }    
    
    if (signum == SIGINT && getpid() == temp)
    {
        int stat;
        while (wait(&stat) > 0);
        printf("Process 2: terminated all child processes\n");
        exit(0);
    }
}

void proc3_handler(int signum, siginfo_t *si, void *ucontext)
{
    
}


int main(int argc, char* argv[])
{
    signal(SIGINT, SIG_IGN);
    int wstat;
    int ppid = getpid();

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = parent_handler;
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &sa, NULL);
    signal(SIGINT, SIG_IGN);
    
    for (int i = 0; i < 3; i++)
    {
        if (ppid == getpid())
        {
            pid[i] = fork();
        }
    }

    // process 1
    if (pid[0] == 0)
    {
        struct sigaction sa0;
        sigemptyset(&sa0.sa_mask);
        sa0.sa_sigaction = proc1_handler;
        sa0.sa_flags = SA_SIGINFO;
        sigaction(SIGUSR1, &sa0, NULL);

        while (1);
    }

    // process 2
    if (pid[1] == 0)
    {   temp = getpid();
        struct sigaction sa1;
        sigemptyset(&sa1.sa_mask);
        sa1.sa_sigaction = proc2_handler;
        sa1.sa_flags = SA_SIGINFO;
        sigaction(SIGUSR1, &sa1, NULL);
        sigaction(SIGINT, &sa1, NULL);
        while (1);
    }


    // process 3
    if (pid[2] == 0)
    {
        signal(SIGINT, SIG_IGN);
        while (1);
    }
    
    //parent
    if (ppid == getpid())
    {
        sleep(1);
        union sigval sv;
        sv.sival_ptr = argv[1];
        printf("Parent: sent string with SIGUSR1 to process1\n");
        sigqueue(pid[0], SIGUSR1, sv);
        for(int i = 0; i < 3; i++) // loop will run n times (n=5) 
        {
            wait(&wstat);
        }

        printf("All child processes (not 3) terminated.\n");
        exit(0);
    }
}