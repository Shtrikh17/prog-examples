#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>

struct command{
    char* command;
    char* arguments[50];
    int argc;
};

enum STATUS{command, arguments_start, arguments_process, trail};
int argc = 0;

struct command* parse(char* input, int* amount){

    struct command* commands = (struct command*) malloc(100*sizeof(struct command));
    int curentCommand = 0;

    int index = 0;
    char* checkpoint = input;
    enum STATUS s = command;

    while(*(input+index)!='\0'){
        if(s==command){
            if(!isalnum(*(input+index))){
                s = arguments_start;
                *(input+index) = '\0';
                commands[curentCommand].command = checkpoint;
                commands[curentCommand].arguments[argc++] = checkpoint;
                checkpoint = input+index+1;
            }
        }
        else if ((s==arguments_process)||(s==arguments_start)){
            if(*(input+index)=='|'){
                s = trail;
                *(input+index) = '\0';
                curentCommand++;
                checkpoint = input+index+1;
                commands[curentCommand].arguments[argc] = NULL;
                argc = 0;
            }
            if(s==arguments_start){
                if(isspace(*(input+index))) {
                    checkpoint += 1;
                }
                else{
                    s = arguments_process;
                }
            }
            if(s==arguments_process){
                if(isspace(*(input+index))) {
                    *(input+index) = '\0';
                    if(checkpoint!='\0')
                        commands[curentCommand].arguments[argc++] = checkpoint;
                    checkpoint = input+index+1;
                }
            }

        }
        else{
            if(isspace(*(input+index))){
                checkpoint += 1;
            }
            else{
                s = command;
            }
        }
        index++;
    }
    if(s == command) {
        commands[curentCommand].command = checkpoint;
        commands[curentCommand].arguments[0] = NULL;
    }
    else{
        if(*checkpoint!='\0')
            commands[curentCommand].arguments[argc++] = checkpoint;
    }
    commands[++curentCommand].command = NULL;
    *amount = curentCommand;
    return commands;
}





int main() {

    char Buffer[1024];
    fgets(Buffer, 1024, stdin);

    int nCommands = 0;
    struct command* commands = parse(Buffer, &nCommands);

    /*
     * pfd[0] - from N to N-1
     * pfd[1] - from N-1 to N-2
     * ...
     * pfd[N-2] - from 1 to 0
     */


    int pipes[nCommands-1][2];             // pipes
    int nPipe = 0;
    for(int i=0; i<nCommands-1; i++)
        pipe(pipes[i]);

    for(int i=0; i<nCommands; i++){

        if(fork()==0){
            // Child Process
            if(nCommands>1) {
                if (i == 0) {
                    close(STDOUT_FILENO);
                    dup2(pipes[nPipe][1], STDOUT_FILENO);
                } else if (i + 1 < nCommands) {
                    close(STDOUT_FILENO);
                    close(STDIN_FILENO);
                    dup2(pipes[nPipe][1], STDOUT_FILENO);
                    dup2(pipes[nPipe-1][0], STDIN_FILENO);
                } else {
                    close(STDIN_FILENO);
                    dup2(pipes[nPipe-1][0], STDIN_FILENO);
                }
            }
            for(int j=0; j<nCommands-1; j++){
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            if(i==nCommands-1){
                int fd = open("/home/shtrikh17/result.out", O_CREAT | O_WRONLY);
                close(STDOUT_FILENO);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(commands[i].command, commands[i].arguments);
        }
        else{
            nPipe++;
        }
    }

    for(int j=0; j<nCommands-1; j++){
        close(pipes[j][0]);
        close(pipes[j][1]);
    }

    while(wait(NULL)>0);
    return 0;
}
