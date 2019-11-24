/****************************************************************
 * Name        : Amari G. Bolmer                                *
 * Class       : CSC 415                                        *
 * Date        : Oct 16, 2018                                   *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include<readline/history.h>
#include<readline/readline.h>
#include <ctype.h>
#include <stdbool.h>
#define BUFFERSIZE 256


// Clearing the shell using escape sequences
#define clear() printf("\e[1;1H\e[2J");

void init_shell(){
    clear();
    printf("\n\n Welcome to MyShell\n");
    printf("\n***********************\n");
    char* username = getenv("USER");
    printf("\n\nWelcome: @%s", username);
    printf("\n");
    sleep(1);

    clear();
}

int takeInput(char* str){
    char* buf;
    buf = readline("");
    if(strlen(buf) != 0){
        add_history(buf);
        strcpy(str, buf);
        return 0;
    }else {
        return 1;
    }
}
void printDir()
{
    char* home = getenv("HOME");
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    
    
    
    if(strcmp(home, cwd) == 0){ //checks if Person is in their home Director
        printf("\nMyShell%s/~", home); // Delete myshell if you want a static path
    }else{
    printf("\nMyShell%s/>>", cwd);//could delete Myshell
    }
}

// Function where the system command is executed
void execArgs(char** parsed){
    int i =0;
    int flagC=0;
    int status=0;
    
    while(parsed[i] != NULL){
        if(strcmp(parsed[i], "&") ==0)
            flagC = 1; //checks for background Process 0 doesnt wait
        i++;
    }
        
    // Forking a child
    pid_t pid = fork();
    
    
    if(flagC==1){
        if((pid=wait(&status))<0){
            perror("wait");
            exit(1);
        }
    }
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate

        wait(NULL);
        
        return;
    }
}


//Function for all piped commands, output and input
void execArgsPiped(char** parsed, char** parsedpipe){
    // 0 is read end, 1 is write end
    int pipefd[2];
    int pid;
    pipe(pipefd);
   
    
    int i =0;
    int flagC=0;
    int status=0;
    
    while(parsed[i] != NULL&& parsedpipe[i] != NULL){
        if(strcmp(parsed[i], "&") ==0)
            flagC = 1; //checks for background Process 0 doesnt wait
        if(strcmp(parsed[i], "&") ==0)
            flagC=1;
        i++;
    }
    // Forking a child
    pid = fork();
    
    if(flagC==1){
        if((pid=wait(&status))<0){
            perror("wait");
            exit(1);
        }
    }
    
    if (pid < 0) {printf("\nCould not fork");return;}
    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);       // Child 1 executing..
        close(pipefd[0]);
        close(pipefd[1]);
        
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        pid = fork();
        if (pid < 0) {
            printf("\nCould not fork");
            return;
        }
        // Child 2 executing..
        // It only needs to read at the read end
        if (pid == 0) {
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[1]);
            close(pipefd[0]);
            
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            int status;
            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid, &status, 0);
        }
    }
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int cmds = 4, i, switchOwnArg = 0;
    char* ownCmds[cmds];
    char* username;
    ownCmds[0] = "exit";
    ownCmds[1] = "cd";
    ownCmds[2] = "help";
    ownCmds[3] = "hello";
    for (i = 0; i < cmds; i++) {
        if (strcmp(parsed[0], ownCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
    switch (switchOwnArg) {
        case 1:
            printf("\nGoodbye\n");
            exit(0);
        case 2:
            chdir(parsed[1]);
            return 1;
        case 3:
            puts("\n***WELCOME TO MYSHELL HELP***"
                 "\n>Use Man pages if need,"
                 "\n>most commands available in UNIX shell"
                 "\n>improper space handling");
            return 1;
        case 4:
            username = getenv("USER");
            printf("\nHello %s. You are using myshell, it may not be perfect but it gets the job done! \n",
                   username);
            return 1;
        default:
            break;
    }
    return 0;
}
//function to change input and redirection
int parseRT(char* str, char** strpiped){
    
    int i=0;
    while((strpiped[i] = strsep(&str, ">")) != NULL){
        if (strpiped[i] == NULL)
            break;
        i++;
    }
    if (strpiped[1] == NULL){ // if no > then return 0
        return 0;
    }else{        // returns 1 if > found.
        return 1;
    }
}
//function to take input and redirection
int parseLT(char* str, char** strpiped){  // <
    
    int i=0;
    while((strpiped[i] = strsep(&str, "<")) != NULL){
        if (strpiped[i] == NULL)
            break;
        i++;
    }
    if (strpiped[1] == NULL){ // if no < then return 0
        return 0;
    }else{        // returns 1 if < found.
        return 1;
    }
}
//funnction to take parse a pipe
int parsePipe(char* str, char** strpiped){
    
    int i=0;
    while((strpiped[i] = strsep(&str, "|")) != NULL){
        if (strpiped[i] == NULL)
            break;
     i++;
    }
    if (strpiped[1] == NULL){ // if no pipe then return 0
        return 0;
    }else{        // returns 1 if pipe found.
    return 1;
    }
}

void parseSpace(char* str, char** parsedP)
{
    int i;
    for (i = 0; i < BUFFERSIZE; i++) {
        
        parsedP[i] = strsep(&str, " "); //strpiped[0] sent in as str, parsing into separate args
        if (parsedP[i] == NULL){
                                        // printf("Break is reached, exiting for loop");
            break;
        }
        if (strlen(parsedP[i]) == 0){ // if zero string, restart at 0 to replace
            i--;
        }
    }
}
int processString(char* str, char** parsed, char** parsedPipe){
    char* strpiped[2];//originallly 2 .
    int piped = 0,LT =0, RT =0;
    
    if((RT+1) == parseRT(str, strpiped)){             //>
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedPipe);
        RT++;
        
    }else if((LT+1) == parseLT(str,strpiped)){        // <
        parseSpace(strpiped[1], parsed);               // Reverse order for redirect
        parseSpace(strpiped[0], parsedPipe);
        LT++;
    }else if((piped+1) == parsePipe(str, strpiped)){      // if pipe has been found
        //printf("PipeHasBeenreached");//Used for debugging pipe
        parseSpace(strpiped[0], parsed);
        parseSpace(strpiped[1], parsedPipe);
        piped++;
    }else {
        parseSpace(str, parsed); //char array holding args being parsed from string
    }
    if(ownCmdHandler(parsed)){
        return 0;
    
    }else{
        return 1+(piped+LT+RT); //If any pipe or redirect found, handle as if pipe
    }
}

int main(void){
    char inputString[BUFFERSIZE], *myargv[BUFFERSIZE];
    char* myArgPiped[BUFFERSIZE];
    
    int Flag = 0;
    init_shell();
    
    while(1){
        printDir();
        if(takeInput(inputString))
                  continue;
        
        Flag = processString(inputString, myargv, myArgPiped);
        
        if(Flag==1){
              execArgs(myargv);
        }
        if(Flag ==2){
            execArgsPiped(myargv, myArgPiped);
        }
    }
    return 0;
}



