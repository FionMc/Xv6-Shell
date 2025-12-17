#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"


/* Print the prompt ">>> " and read a line of characters
   from stdin. */
int getcmd(char *buf, int nbuf) {
  // ##### Place your code here
  int num;
  do { 
    printf(">>> ");
    num = read(0, buf, nbuf);
  } while (num<=1);

return 0;
}

/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn))
void run_command(char *buf, int nbuf, int *pcp) {
  /* Useful data structures and flags. */
  char *arguments[10];
  int numargs = 0;
  /* Flags to mark word start/end */
  int ws = 1;
  //int we = 0;

  /* Flags to mark redirection direction */
  int redirection_left = 0;
  //< = left
  int redirection_right = 0;
  //> = right

  /* File names supplied in the command */
  char *file_name_l = 0;
  char *file_name_r = 0;

  //int p[2];
  int pipe_cmd = 0;
  int leftside = 0;
  int pipeposition=0;

  /* Flag to mark sequence command */
  int sequence_cmd = 0;

  //Finds length of stdin to use in loop
    int length = 0;
    char *temp = buf;
    while (*temp != '\0') {
        length++;
        temp++;
    }

  /* Parse the command character by character. */
  for (int i = 0; i < length; i++) {
    if(buf[i] == '<') {
      redirection_left = 1;
    }
    if(buf[i] == '>') {
      redirection_right = 1;
    }
    /* Parse the current character and set-up various flags:
       sequence_cmd, redirection, pipe_cmd and similar. */
    /* ##### Place your code here. */
    if(buf[i] != '\n') {
      if(buf[i] != '\0') {
        if(buf[i] != ' ') {
          if(buf[i] != '<') {
            if(buf[i] != '>') {
              if(buf[i] != '|') {
                if(ws) {
                  arguments[numargs] = &buf[i];
                  ws=0;
                  numargs++;
                } 
              } else {
                pipe_cmd = 1;
                pipeposition = i;
                arguments[numargs] = '\0';
                break;
              }
            }
            else {
              redirection_right = 1;
            }
          }
          else {
            redirection_left = 1;
          }
        } else if(ws==0 && buf[i] == ' ') { //space
            buf[i] = '\0';
            ws = 1;
          } else { 
            i=i;
          }
    }
    } else { //\n
        buf[i] = '\0';
        arguments[numargs] = '\0';
        break;
      }

    if (!(redirection_left || redirection_right)) {
      /* No redirection, continue parsing command. */
      // Place your code here.
    } else {
      /* Redirection command. Capture the file names. */
      // In for loop still so remember loops for all characters!
      // ##### Place your code here.
      if(buf[i-2] == '<'){
        file_name_l = &buf[i];
      }
      if(buf[i-2] == '>'){
        file_name_r = &buf[i];
      }
    }
  }
 
  /*
    Sequence command. Continue this command in a new process.
    Wait for it to complete and execute the command following ';'.
  */
  if (sequence_cmd) {
    sequence_cmd = 0;
    if (fork() != 0) {
      wait(0);
      // ##### Place your code here.
      // Call run_command recursively
    }
  }

  /*
    If this is a redirection command,
    tie the specified files to std in/out.
  */
  if (redirection_left) {
    // ##### Place your code here.
    //Remove file name from arguments
    arguments[numargs-1] = arguments[numargs];
    close(0);
    open(file_name_l, O_RDONLY);
  }
  
  if (redirection_right) {
    // ##### Place your code here.
    //Remove file name from arguments
    arguments[numargs-1] = arguments[numargs];
    close(1);
    open(file_name_r, O_WRONLY | O_CREATE | O_TRUNC);
  }

  /* Parsing done. Execute the command. */
  

  /*
    If this command is a CD command, write the arguments to the pcp pipe
    and exit with '2' to tell the parent process about this.
  */

  if (strcmp(arguments[0], "cd") == 0) {
    // ##### Place your code here.
      close(pcp[0]);
      printf("\n arg[1] = %s", arguments[1]);
      printf("\n arg[1] len = %d", strlen(arguments[1]));
      int result = write(pcp[1], arguments[1], sizeof(arguments[1]));
      printf("result = %d", result);
      
    exit(2);
  } else {
    /*
      Pipe command: fork twice. Execute the left hand side directly.
      Call run_command recursion for the right side of the pipe.
    */
    if (pipe_cmd) {
      // ##### Place your code here
      int p[2];
      pipe(p);
      //LHS
      if(fork() == 0) {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        exec(arguments[0], arguments);
      }

      if(fork() == 0) {
        //Now do right hand side
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        run_command(&buf[pipeposition+1],50,p);
      }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);
    } else {
      // ##### Place your code here.
      // Simple command; call exec()
      exec(arguments[0], arguments);
    }
  }
exit(0);
}

int main(void) {

  static char buf[100];
  int pcp[2];
  pipe(pcp);


  /* Read and run input commands. */
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(fork() == 0) 
      run_command(buf, 100, pcp);
    
    /*
      Check if run_command found this is
      a CD command and run it if required.
    */

    // ##### Place your code here
    /*Place argument/command into exec to run
    exec();*/
   
    int child_status;
    wait(&child_status);
    char temp[10];

    if(child_status == 2) {
        //close(pcp[1]);
        int readresult = read(pcp[0], temp, sizeof(temp));
        printf("\n read result = %d", readresult);
        printf("\nread from buffer = %s", temp);
        chdir(temp);
      } 
  }
  exit(0);
}
