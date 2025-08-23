/************************************************  
@file main.c
@auther kerolos_marcellus
*************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define LSH_RL_BUFSIZE  1024  // lsh_read_line_buffer_size
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM   " \t\r\n\a"

void lsh_loop();
char* lsh_read_line(void);
char** lsh_split_line(char*);
int lsh_execute(char** args);

/*
    Function Declarations for builtin shell commands
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char ** args);

/*
    List of built in commands followed by their corresponding functions.
*/
char* builtin_str[] = {
    "cd", 
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};
// this is an array of functions pointer
// an array of functions that takes (char **) as thier parameters
// and return an int
// each element will point to the function that handles the command

// a helper function that calculate how many commands exist by dividing the total size of the 
// array by the size of one element
int lsh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}


int main(int argc, char** argv){
    // load any config files


    // run the command loop
    lsh_loop();

    // perform any shutdown or cleanup

    return EXIT_SUCCESS;
    
}
void lsh_loop(){
    char* line;
    char** args;
    int status;

    do{
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
         status = lsh_execute(args);

        free(line);
        free(args);

    }while(status);
}

char* lsh_read_line(void){
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufsize);
    int c ; // not a char because EOF is an int not a char.

    if(!buffer){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        // read a character
        c = getchar();

        // if we hit EOF, replace it will a null and return .
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;

        // if you exceeded the buffer reallocate
        if(position >= bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char** lsh_split_line(char* line){
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char** tokens = malloc(sizeof(char*) * bufsize);
    char* token;

    if(!tokens){
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    // strtok will return a pointer to the first token and place \0 at the end of each token
    while(token != NULL){
        tokens[position] = token;
        ++position;

        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            char** new_tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!new_tokens){
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char** args){
    pid_t pid, wpid;
    int status;             // status for the child process

    pid = fork();
    if(pid == 0){
        // child process
        /*  exec --> execute
            v    --> vector (array of arguments)
            p    --> path   (searches in PATH environment variable)
        
        */

        if(execvp(args[0], args) == -1){
            perror("lsh");              // like fprintf but it writes alywas to stderr
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0){
        // error forking
        perror("lsh");
    }else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);  
            /*
                status is a pointer where status information is stored
                
                wpid --> process pid of the child that changed state
                
                WUNTRACED   W-> Wait(part of the wait system call)
                WUNTRACED tells the waitpid() to also return when a child process is stopped (not just when it exits)
                without WUNTRACED flag, waitpid() will return when child exits or is killed
                
                UnTraced    --> not traced/ stopped
            */
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
        /* 
            WIFEXITED() --> True --> child called exit() or return from main()
                            False--> child was killed by a signal or stopped

            WIFSIGNALED()-> True --> child was terminated by a signal(like SIGKILL, SIGTERM)
                            False -> child exited normally or was stopped
            
            the loop Translate to 
            "Keep looping while the child has NOT exited normally AND has not beed killed by a signal"
        */
    }   
    return 1;
}

int lsh_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "Lsh: Expected argument to \"cd\" \n");
    }else{
        if(chdir(args[1]) != 0){
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help (char** args){
    int i;
    printf("Kerolos Marcellus's LSH\n");
    printf("Type the program name and arguments, then hit enter. \n");
    printf("The following are built in: \n");

    for(i = 0; i < lsh_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for informations on other programs. \n");
    return 1;
}

int lsh_exit(char ** args){
    return 0;
}

int lsh_execute(char** args){
    int i ;
    if(args[0] == NULL){
        // an empty command
        return 1;
    }

    for(i = 0; i < lsh_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return lsh_launch(args);
}














