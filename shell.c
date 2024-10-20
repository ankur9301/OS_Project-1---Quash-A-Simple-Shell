#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
int child_pid;

void terminateSignalHandler(int signum){
  printf("\n");
}

void alarmSignalHandler(int signum) {
  kill(child_pid, SIGINT);
}

int main() {
    signal(SIGINT, terminateSignalHandler);
    signal(SIGALRM, alarmSignalHandler);

    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];
    	
    while (true) {
        char currDir[MAX_COMMAND_LINE_LEN];

        do{ 
            // Print the shell prompt.
            getcwd(currDir, sizeof(currDir));
            printf("%s>", currDir);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed

        // 1. Tokenize the command line input (split it on whitespace)
  
        int command_count = 0;
        char *input_token = strtok(command_line, delimiters);   //strtok tokenizes the input
        while (input_token != NULL && command_count < MAX_COMMAND_LINE_ARGS) {
            if (input_token[0] == '$') {
                char *env_value = getenv(input_token+1);
                if (env_value != NULL) {
                  arguments[command_count++] = getenv(input_token+1);
                }
            }   
            else{
                arguments[command_count++] = input_token;
            }
            input_token = strtok(NULL, delimiters);
        }

        int pid;
        int bg_execution = 0;
        if (command_count > 1){
          if (strcmp(arguments[command_count - 1], "&") == 0){
            arguments[command_count --] = NULL;
            pid = fork();
            if (pid == -1){
              perror("Error forking process");
            }
            else if (pid == 0){
            setsid();
            if (command_count == 1) {
                if(execlp(arguments[0], arguments[0], NULL) == -1){
                  perror("Error Executing command");
                  exit(EXIT_FAILURE);
                }
                return;
            } else {
                if(execvp(arguments[0], arguments) == -1){
                  perror("Error Executing command");
                  exit(EXIT_FAILURE);
                }
                return;
            }
            exit (0);
          } else {
            bg_execution = 1;
          }
        }
        }
        if (bg_execution == 1){
          continue;
        }

        // 2. Implement Built-In Commands
        char *user_input = arguments[0];
        if(strcmp(user_input, "cd") == 0){
          if(command_count == 2){
              int change_dir_result = chdir(arguments[1]);
              if (change_dir_result != 0){
                perror("Failed to change directory");
              } 
          }
          else {
            printf("Invalid syntax: Please use cd <directory path> \n");
          }
        } 
        else if(strcmp(user_input, "pwd") == 0){
            printf("%s \n", currDir);
        } 
        else if(strcmp(user_input, "echo") == 0){
            int i;
            for(i = 1; i < command_count; i++){
                printf("%s ",arguments[i]);
            }
            printf("\n");
        } 
        else if(strcmp(user_input, "exit") == 0){
            exit(0);
        } 
        else if(strcmp(user_input, "env") == 0){
            if(command_count == 1){
              char **env = environ;
                while (*env != NULL) {
                printf("%s\n", *env);
                env++;
              }
            }
            else if(command_count == 2) {
              char *env_value = getenv(arguments[1]);
              printf("%s \n", env_value);
            } 
            else {
              printf("Invalid syntax: Please use env or env <variable name> \n");
            }
        } 
        else if(strcmp(user_input, "setenv") == 0){
            if (command_count == 3){
              if(setenv(arguments[1], arguments[2], 1) != 0){
                perror("env");
              }
            } else {
              printf("Invalid syntax: Please use setenv <variable name> <variable value>");
            }
        }
        else {

            // 3. Create a child process which will execute the command line input
            child_pid = fork();

            if (child_pid == -1) {
              perror("Error forking");
            }
            else if (child_pid == 0){
              alarm(10);
              if (command_count == 1){
                if (execlp(arguments[0], arguments[0], NULL) == -1) {
                  
                  perror("Error executing command");
                  exit(EXIT_FAILURE);
                }
              }
              else  {
                if (execvp(arguments[0], arguments) == -1){ 
                
                perror("Error executing command");
                exit(EXIT_FAILURE);
              }
              }
              int i;
                for(i = 0; i < command_count;i++){
                arguments[i] = NULL;
              }
              exit(0);
            } else {
              int status;
              wait(&status);          
              int i;
                for(i = 0; i < command_count;i++){
                arguments[i] = NULL;
              }
              alarm(0);
            }

        }
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

    }
    // This should never be reached.
    return -1;
}

