#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "vect.h"
#include "token.h"

const size_t buffer_limit = 512;
int status;
void runCommand(vect_t *tokens);
void cd(vect_t *tokens);
void helpCmd(vect_t *tokens);
void source(vect_t *tokens);

void pipeFunc(vect_t **tokenList);
void outputRedirect(vect_t **tokenList);
void inputRedirect(vect_t **tokenList);
void sequence(vect_t **tokenList);



int main(int argc, char **argv) {
  char *buffer;
  char welcome[] = "Welcome to mini-shell.\n";
  char startMsg[] = "shell $ ";

  status = 0;
  vect_t *prev_cmd = NULL;
  assert(write(1, welcome, strlen(welcome)) == strlen(welcome));

  buffer = (char *) malloc(buffer_limit * sizeof(char));

  // While there is no exit or ctrl-d, run the shell
  while(status == 0) {

    assert(write(1, startMsg, strlen(startMsg)) == strlen(startMsg));

    size_t length = getline(&buffer, &buffer_limit, stdin);

    if (length == -1) {
      break;
    }

    // Make sure the stdin is more than 1 character and less than 512 bytes
    if(strlen(buffer) >= buffer_limit) {
      char *tooManyChar = "Please enter less than 512 characters\n";
      assert(write(1, tooManyChar, strlen(tooManyChar)) == strlen(tooManyChar));
      continue;
    }

    // For reading in files this might be different 
    vect_t *tokens = parseInput(buffer);

    // If there are no arguments, continue to the next iteration
    if (vect_size(tokens) <= 0) {
      vect_delete(tokens);
      continue;
    }

    // Check if the command is prev
    if(strcmp(vect_get(tokens, 0), "prev") == 0){
      // Check if its the first iteration so there is no prev
      if(prev_cmd == NULL) {
	char prevError[] = "There is no previous command\n";
	assert(write(1, prevError, strlen(prevError)) == strlen(prevError));
	continue;
      }
      else {
	runCommand(prev_cmd);
	vect_delete(tokens);
	continue; // Doesn't store the "prev" command in prev_cmd
      }
    }

    // Run the command with the tokens
    runCommand(tokens);

    // Storing the previous command
    prev_cmd = copy_vect(prev_cmd, tokens);

    vect_delete(tokens);
  }

  vect_delete(prev_cmd);
  free(buffer);
  return 0;
}


// Function to process the built in commands
void processBuiltIn(vect_t *tokens){
  // exit case
  if(strcmp(vect_get(tokens, 0), "exit") == 0){
    char bye[] = "Bye bye.\n";
    assert(write(1, bye, strlen(bye)) == strlen(bye));
    status = 1;
  }

  // cd case
  else if(strcmp(vect_get(tokens, 0), "cd") == 0){
    cd(tokens);
    return;
  }

  // source case
  else if(strcmp(vect_get(tokens, 0), "source") == 0){
    source(tokens);
  }

  //help case
  else if(strcmp(vect_get(tokens, 0), "help") == 0 && vect_size(tokens) == 1){
    helpCmd(tokens);
  }
}

// checks to see if the command is a built in
int isBuiltIn(vect_t *tokens){
  // exit case
  if(strcmp(vect_get(tokens, 0), "exit") == 0){
    return 1;
  }

  // cd case
  if(strcmp(vect_get(tokens, 0), "cd") == 0){
    return 1;
  }

  // source case
  if(strcmp(vect_get(tokens, 0), "source") == 0){
    return 1;
  }

  //help case
  if(strcmp(vect_get(tokens, 0), "help") == 0 && vect_size(tokens) == 1){
    return 1;
  }

  return 0;
}

// Checks if it contains a special token
int containsSpecial(vect_t *tokens){
  for(int i = 0; i < vect_size(tokens); i++){

    if(strcmp(vect_get(tokens, i), "|") == 0 
	|| strcmp(vect_get(tokens, i), "<") == 0 
	|| strcmp(vect_get(tokens, i), ">") == 0 
	|| strcmp(vect_get(tokens, i), ";") == 0){
      return 1;
    }
  }

  return 0;
}

// Method to process special characters in the tokens
void processSpecial(vect_t *tokens){
  // Case where there is a pipe
  // Split tokens into before the pipe and after and pass it to pipeFunc
  int pipeIdx = indexOf(tokens, "|");

  if(pipeIdx != -1){
    vect_t *tokenList[2];
    tokenList[0] = copy_vect_until(NULL, tokens, pipeIdx);
    if(vect_size(tokens) - 1 <= pipeIdx){
      runCommand(tokenList[0]);
      vect_delete(tokenList[0]);
      return;
    } 
    tokenList[1] = copy_vect_after(NULL, tokens, pipeIdx + 1);
    pipeFunc(tokenList);
    //free(tokenList);
    return;
  }

  // unsure if pipe or sequence should come first
  // Case where there is sequencing
  // Split at ; and pass to sequence
  int sequenceIdx = indexOf(tokens, ";");
  if(sequenceIdx != -1){
    vect_t *tokenList[2];
    tokenList[0] = copy_vect_until(NULL, tokens, sequenceIdx);
    if(vect_size(tokens) - 1 <= sequenceIdx){
       runCommand(tokenList[0]);
       vect_delete(tokenList[0]);
       return;
     }
    tokenList[1] = copy_vect_after(NULL, tokens, sequenceIdx+1);
    sequence(tokenList);
    return;
  }

  // Case where this is a output redirection
  // Split the tokens to before the > and after and pass to outputRedirection
  int outputIdx = indexOf(tokens, ">");
  if(outputIdx != -1){
    vect_t *tokenList[2];
    tokenList[0] = copy_vect_until(NULL, tokens, outputIdx);
    if(vect_size(tokens) - 1 <= outputIdx){
       runCommand(tokenList[0]);
       vect_delete(tokenList[0]);
       return;
    }
    tokenList[1] = copy_vect_after(NULL, tokens, outputIdx+1);
    outputRedirect(tokenList);
    return;
  }

  // Case where there is input redirection
  // Split at < and pass to inputRedirection
  int inputIdx = indexOf(tokens, "<");
  if(inputIdx != -1){
    vect_t *tokenList[2];
    tokenList[0] = copy_vect_until(NULL, tokens, inputIdx);
    if(vect_size(tokens) - 1 <= inputIdx){
       runCommand(tokenList[0]);
       vect_delete(tokenList[0]);
       return;
     }
    tokenList[1] = copy_vect_after(NULL, tokens, inputIdx+1);
    inputRedirect(tokenList);
    return;
  }
}

// Function that does the pipe functionality
void pipeFunc(vect_t **tokenList){
  int pipe_fd[2];

  //create the pipe
  assert(pipe(pipe_fd) == 0);

  // Assign each end of the pipe
  int readEnd = pipe_fd[0];
  int writeEnd = pipe_fd[1];

  // save stdin and stdout
  int saved_stdout = dup(1);
  int saved_stdin = dup(0);

  // Fork child A
  int childa_pid = fork();
  int status;

  // In child
  if(childa_pid == 0){
    close(readEnd);

    //close stdout
    if (close(1) == -1) {
      perror("Error closing stdout");
      exit(1);
    }

    // replace stdout with write end of the pipe
    dup2(writeEnd, STDOUT_FILENO);

    // run the first command
    runCommand(tokenList[0]);

    vect_delete(tokenList[0]);
    vect_delete(tokenList[1]);
    //free(tokenList);
    //exit the child
    exit(0);  
  }
  // In parent
  else if(childa_pid > 0){
    // wait for child to finish
    waitpid(childa_pid, &status, 0);
    close(writeEnd);

    // close stdin
    if(close(0) == -1){
      perror("Error closing stdin");
      exit(1);
    }

    // replace stdin with the read end
    dup2(readEnd, STDIN_FILENO);

  }
  else {
    perror("Error - fork failed");
    exit(1);
  }

  int status2;
  // Fork child B
  int childb_pid = fork();

  // IN child B
  if(childb_pid == 0){
    close(writeEnd);

    // Put stdout back where it needs to be
    dup2(saved_stdout, STDOUT_FILENO);

    // run the second command with stdin being read end of pipe
    // and stdout being stdout
    runCommand(tokenList[1]);

    vect_delete(tokenList[0]);
    vect_delete(tokenList[1]);
    // exit child
    //free(tokenList);
    exit(0);
  }
  // In parent B
  else if(childb_pid > 0){
    // Wait for child
    waitpid(childb_pid, &status2, 0);

    // Put stdin back
    dup2(saved_stdin, STDIN_FILENO);

  }
  else {
    perror("Error - fork failed");
    exit(1);
  }

  vect_delete(tokenList[0]);
  vect_delete(tokenList[1]);
}


// Method to redirect the output
void outputRedirect(vect_t **tokenList){
 // save stdout so it can be put back where it belongs
  int saved_stdout = dup(1);

  // close stdout
  if (close(1) == -1) {
    perror("Error closing stdout");
  }

  // open the given file for writing and creating if the file doesn't exist
  // If error put stdout back and print error
  int fd = open(vect_get(tokenList[1],0) , O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    dup2(saved_stdout, STDOUT_FILENO);
    perror("Error trying to open file");
    return;
  }

  // replace stdout with the file
  dup2(fd, STDOUT_FILENO);

  // Run the command with stdout being the file
  runCommand(tokenList[0]);

  // Close the file
  close(fd);

  // Put stdout beack
  dup2(saved_stdout, STDOUT_FILENO);
  vect_delete(tokenList[0]);
  vect_delete(tokenList[1]);
}

// Method to redirect the input of a command
void inputRedirect(vect_t **tokenList){
  // Save stdin for later use
  int saved_stdin = dup(0);

  // close stdin
  if (close(0) == -1) {
    perror("Error closing stdin");
  }

  // Open the file for reading
  // Put stdin back then do the error
  int fd = open(vect_get(tokenList[1],0) ,O_RDONLY);
  if (fd == -1) {
    dup2(saved_stdin, STDIN_FILENO);
    perror("Error trying to open file");
    return;
  }

  // Make the file stdin
  dup2(fd, STDIN_FILENO);

  // Run the command with stdin as the file
  runCommand(tokenList[0]);

  // Close the file
  close(fd);

  // Put stdin back
  dup2(saved_stdin, STDIN_FILENO);
  vect_delete(tokenList[0]);
  vect_delete(tokenList[1]);
}

// Method to sequence two commands
void sequence(vect_t **tokenList){
  runCommand(tokenList[0]);
  runCommand(tokenList[1]);
  vect_delete(tokenList[0]);
  vect_delete(tokenList[1]);
}

// Method to run a command
void runCommand(vect_t *tokens){
  // Case where the command is a built in
  if(isBuiltIn(tokens) == 1){
    processBuiltIn(tokens); 
  }

  // Case where the command has special characters that in it 
  else if(containsSpecial(tokens) == 1){
    processSpecial(tokens);
  }

  // Case where the command is in bin
  else {
    // Make the first arg have /bin/ in front for exec
    char *args[vect_size(tokens)+1];
    char *executable = (char *)malloc(strlen("/bin/") + strlen(vect_get(tokens, 0)) + 1);
    strcpy(executable, "/bin/");  // Copy "/bin/"
    strcat(executable, vect_get(tokens, 0));  // Concatenate the command
    args[0] = executable;

    // Create the not found string incase the command doesn't exist
    char *notFound = (char *)malloc(strlen(" : command not found\n") + strlen(vect_get(tokens, 0)) + 1);
    strcpy(notFound, vect_get(tokens,0));
    strcat(notFound, " : command not found\n");  // Concatenate the command

    // Copy the tokens to a char[] for exec
    for(int i = 1; i < vect_size(tokens); i++) {
      args[i] = (char *) malloc(strlen(vect_get(tokens,i))+1);
      strcpy(args[i],vect_get(tokens,i));
    }

    // set last arg to null for exec
    args[vect_size(tokens)] = NULL;

    // Fork to run comand
    if (fork()  == 0) {
      // Executes command from child then terminates our process
      // Note: There are other 'exec' functions that may be helpful. Look at 'man exec' and 'man execve'.
      execve(args[0], args, NULL);

      // If reached there was an error
      assert(write(1, notFound, strlen(notFound)) == strlen(notFound));
      //perror("execvp");
      exit(1);              
    }

    // Wait till child is finished
    wait(NULL);

    // Free the allocated memory
    free(executable);
    for(int i = 1; i < vect_size(tokens); i++){
      free(args[i]);
    }
    free(notFound);
  }
}

// Function for the cd command
void cd(vect_t *tokens){
  // case where there are too many args to cd
  if (vect_size(tokens) != 2) {
    char tooArg[] = "cd: too many args\n";
    assert(write(1, tooArg, strlen(tooArg)) == strlen(tooArg));
    return;
  }

  // Get the directory we are changing to
  const char *newDir = vect_get(tokens, 1);

  // Change directory faiiled
  if (chdir(newDir) != 0) {
    perror("cd");
  }
}

void source(vect_t *tokens){

  // Checks if the no. of arguments is correct
  if (vect_size(tokens) != 2) {
    char oneArg[] = "source: only 1 args expected\n";
    assert(write(1, oneArg, strlen(oneArg)) == strlen(oneArg));
    return;
  }

  // Opens the file to read from it
  const char *file_path = vect_get(tokens, 1);
  FILE *fd = fopen(file_path, "r");
  if (fd == NULL) {
    perror("Error reading file");
    return;
  }

  size_t buff_size = 0;
  char *buffer = NULL;
  size_t length = 0;
  fpos_t *pos = (fpos_t *)malloc(sizeof(fpos_t));

  length = getline(&buffer, &buff_size, fd);
  vect_t *prev_cmd = NULL;
  fgetpos(fd,pos);
  while(length != -1) {
    vect_t *tokens = parseInput(buffer);

    // Check if the command is prev
    if(strcmp(vect_get(tokens, 0), "prev") == 0){
      // Check if its the first iteration so there is no prev
      if(prev_cmd == NULL) {
	char prevError[] = "There is no previous command\n";
	assert(write(1, prevError, strlen(prevError)) == strlen(prevError));
	vect_delete(tokens);
	continue;
      }
      else {
	fsetpos(fd,pos);
	runCommand(prev_cmd);
	length = getline(&buffer, &buff_size, fd);
	fgetpos(fd,pos);
	vect_delete(tokens);
	continue; // Doesn't store the "prev" command in prev_cmd
      }
    }

    fsetpos(fd,pos);
    runCommand(tokens);

    // Storing the previous command
    prev_cmd = copy_vect(prev_cmd, tokens);

    free(buffer);
    length = getline(&buffer, &buff_size, fd);
    fgetpos(fd,pos);
    vect_delete(tokens);
  }

  vect_delete(prev_cmd);
  free(pos);
}

void helpCmd(vect_t *tokens){
  char *helpMsg = "cd: Change the shell working directory.\npwd: Print the name of the current working directory.\nhelp:  Display information about builtin commands.\nprev: Runs the previous command, not including itself";

  assert(write(1, helpMsg, strlen(helpMsg)) == strlen(helpMsg));
  return;

}

