#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define MAXLINE 200  /* This is how we declare constants in C */
#define MAXARGS 20
#define STDIN 0
#define STDOUT 1

int pipe_fd[2];
int IS_PIPE = 0;
int INP_REDIR = 0;
int OUT_REDIR = 0;
int fd;

/* In C, "static" means not visible outside of file.  This is different
 * from the usage of "static" in Java.
 * Note that end_ptr is an output parameter.
 */

static char * getword(char * begin, char **end_ptr) {
    char * end = begin;

    while ( *begin == ' ' )
        begin++;  /* Get rid of leading spaces. */
    end = begin;
    while ( *end != '\0' && *end != '\n' && *end != ' ' )
        end++;  /* Keep going. */
    if ( end == begin )
        return NULL;  /* if no more words, return NULL */
    *end = '\0';  /* else put string terminator at end of this word. */
    *end_ptr = end;
    if (begin[0] == '$') { /* if this is a variable to be expanded */
        begin = getenv(begin+1); /* begin+1, to skip past '$' */
        if (begin == NULL) {
            perror("getenv");
            begin = "UNDEFINED";
        }
    }
    return begin; /* This word is now a null-terminated string.  return it. */
}

/* In C, "int" is used instead of "bool", and "0" means false, any
 * non-zero number (traditionally "1") means true.
 */
/* argc is _count_ of args (*argcp == argc); argv is array of arg _values_*/
static void getargs(char cmd[], int *argcp, char *argv[])
{
    char *cmdp = cmd;
    char *end;
    int i = 0;
    /* fgets creates null-terminated string. stdin is pre-defined C constant
     *   for standard intput.  feof(stdin) tests for file:end-of-file.
     */
    if (fgets(cmd, MAXLINE, stdin) == NULL && feof(stdin)) {
        printf("Couldn't read from standard input. \n  End of file? Exiting ...\n");
        exit(1);  /* any non-zero value for exit means failure. */
    }
    while ( (cmdp = getword(cmdp, &end)) != NULL && cmdp[0] != '#') { /* end is output param */
        /* getword conver
         * ts word into null-terminated string */
        argv[i++] = cmdp;
        /* "end" brings us only to the '\0' at end of string */
        cmdp = end + 1;
        if (cmdp[0] == '|') {
            IS_PIPE = 1;
        }

        if (cmdp[0] == '>') {
            OUT_REDIR = 1;
        }

        if (cmdp[0] == '<') {
            INP_REDIR = 1;
        }
    }
    argv[i] = NULL; /* Create additional null word at end for safety. */
    *argcp = i;

}
static void execute(int argc, char *argv[])
{
    int i; // global variable to find index of the '>', '<', '|'
    pid_t childpid; /* child process ID */

    if (IS_PIPE) {
        IS_PIPE = 0;      
        // After the pipe identified, turn the pipe's flag 
        // again to false
        int pipe_idx; // the position of the pipe

        // find the position of the pipe
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "|") == 0) {
                pipe_idx = i;
                break;
            }
        } 
        // create a pipeline
        pipe(pipe_fd);
        
        // save the argument right behind the pipe
        char* snd = argv[pipe_idx + 1];
        // fork
        pid_t thispid = fork();
        if (thispid == 0) {
            if ( -1 == close(STDOUT) ) perror("close"); /*close*/
            fd = dup(pipe_fd[1]); /*set up empty STDOUT to be pipe_fd[1] */
            if (-1 == fd ) perror("dup");
            if ( fd != STDOUT) fprintf(stderr, "Pipe output not at STDOUT.\n");
            close(pipe_fd[0]); // never used
            close(pipe_fd[1]); // not needed anymore

            for (i = pipe_idx; i < argc; i++) {
                argv[i] = NULL;
            }
            // execute
            if ( -1 == execvp(argv[0], argv) ) perror("execvp");
        } else if (fork() == 0) {
            if ( -1 == close(STDIN) ) perror("close");
            fd = dup(pipe_fd[0]);
            if ( -1 == fd ) perror("dup");
            if (fd != STDIN) fprintf(stderr, "Pipe input not at STDIN.\n");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            // find index of the pipe and put the NULL
            argv[0] = snd;
            // only execute the snd command, don't need to use the pipe index
            for (i = 1; i < argc; i++) {
                argv[i] = NULL;
            }
            // execute
            if ( -1 == execvp(argv[0], argv) ) perror("execvp");
        } else {
            int status;
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            waitpid(thispid, &status, 0);
            waitpid(thispid, &status, 0);
        }
    }

    else if (INP_REDIR) {
        INP_REDIR = 0;
        // after found the input redirection,
        // change the flag back to false
        int redir_idx; // find the index of the redirection symbol
        
        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], "<") == 0) {
                redir_idx = i;
                break;
            }
        }

        char* filename = argv[redir_idx + 1]; // the file name

        pid_t pid = fork();

        if (pid == -1) {
            perror("no child!");
        } else if (pid == 0) {
            close(STDIN);
            fd = open(filename, O_RDONLY);
            if (fd == -1)
                perror("OPEN FOR READING");
            for (i = redir_idx; i < argc; i++) {
                // set every argv from the redir_inp to be NULL
                argv[i] = NULL;
            }
            execvp(argv[0], argv); 
        } else {
            waitpid(pid, NULL, 0);
        }
    }

    else if (OUT_REDIR) {
        OUT_REDIR = 0;
        // after found the output redirection,
        // change the flag back to false
        int redir_out_idx; 
        
        for (i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") == 0) {
                redir_out_idx = i;
                break;
            }
        }

        char* filename = argv[redir_out_idx + 1]; //the file name
        pid_t pid = fork();
        if (pid == -1) {
            perror("no child!");
        } else if (pid == 0) {
            close(STDOUT);
            fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
            if (fd == -1) 
                perror("OPEN FOR WRITING");
            for (i = redir_out_idx; i < argc; i++) {
                argv[redir_out_idx] = NULL;
            }
            execvp(argv[0], argv);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
    else {
        childpid = fork();
        if (childpid == -1) { /* in parent (returned error) */
            perror("fork"); /* perror => print error string of last system call */
            printf("  (failed to execute command)\n");
        }
        if (childpid == 0) { /* child:  in child, childpid was set to 0 */
            /* Executes command in argv[0];  It searches for that file in
             *  the directories specified by the environment variable PATH.
             */

            if (-1 == execvp(argv[0], argv)) {
                perror("execvp");
                printf("  (couldn't find command)\n");
            }
            /* NOT REACHED unless error occurred */
            exit(1);
        } else /* parent:  in parent, childpid was set to pid of child process */
            waitpid(childpid, NULL, 0);  /* wait until child process finishes */
    }
    return;
}

void interrupt_handler(int signum){
    printf("\n");
    //fflush(stdout);
    if (signum == 1) {
        kill(signum, SIGINT);
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, interrupt_handler);

    char cmd[MAXLINE];
    char *childargv[MAXARGS];
    int childargc;

    if (argc > 1) {
        freopen(argv[1], "r", stdin);
    }

    while (1) {
        printf("%% "); /* printf uses %d, %s, %x, etc.  See 'man 3 printf' */
        fflush(stdout); /* flush from output buffer to terminal itself */
        getargs(cmd, &childargc, childargv); /* childargc and childargv are
                                                output args; on input they have garbage, but getargs sets them. */
        /* Check first for built-in commands. */
        if ( childargc > 0 && strcmp(childargv[0], "exit") == 0 )
            exit(0);
        else if ( childargc > 0 && strcmp(childargv[0], "logout") == 0 )
            exit(0);
        else
            execute(childargc, childargv);
    }
}
