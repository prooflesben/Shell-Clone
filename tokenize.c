#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vect.h"

// Tokenizes the inputs
vect_t *parseInput2();

// Handles cases that are in quotes
int handle_string2(int i, char *input, vect_t *output);

// Checks if the given character is a special character
int isSpecialChar2(char c);

int main(int argc, char **argv) {
  vect_t *output = parseInput2();

  for(int i = 0; i < vect_size(output); i++){
    printf("%s\n", vect_get(output, i));
  }

  vect_delete(output);
  return 0;
}

// Checks if the given character is a special character
int isSpecialChar2(char c) {
  if (c == '(' || c == ')'
      || c == '<' || c == '>'
      || c == ';' || c == '|'
      || c == ' ') {
    return 1;
  }
  return 0;
}

vect_t *parseInput2() {
  char *input = (char *) malloc(255);
  fgets(input, 255, stdin);

  // Keeps track of the string not seperated by space
  char *buffer = (char *) malloc(255);

  // Keeps track of the last character within the string
  int bufferIdx = 0;

  vect_t *output = vect_new();
  for(int i = 0; i < strlen(input); i++) {
    char curChar = input[i];

    // Checks if a special case was encountered to add all the temporarily stored string into the vector
    if (bufferIdx > 0 
	&& (isSpecialChar2(curChar)
	  || (curChar == '\\' && (input[i+1] == 'n' ||  input[i+1] == 't'))
	  || (curChar == '\"' || (curChar == '\\' && input[i+1] == '\"')) || curChar == '\n' || curChar == '\t')) {
      buffer[bufferIdx] = '\0';
      vect_add(output, buffer);
      bufferIdx = 0;
    }
    else if (i == strlen(input)-1
	&& !(isSpecialChar2(curChar)
	  || (curChar == '\\' && (input[i+1] == 'n' ||  input[i+1] == 't'))
	  || (curChar == '\"' || (curChar == '\\' && input[i+1] == '\"')) || curChar == '\n' || curChar == '\t')) {
      buffer[bufferIdx] = curChar;
      buffer[bufferIdx +1] = '\0';
      bufferIdx = 0;
      vect_add(output, buffer);
    }

    // Checks if the character on its own is a token
    if (curChar == '(' || curChar == ')' 
	|| curChar == '<' || curChar == '>' 
	|| curChar == ';'  || curChar == '|') {
      vect_add(output, &curChar);
      continue;
    }

    // Checks if it is an enter or a tab
    if ((curChar == '\\' && (input[i+1] == 'n' ||  input[i+1] == 't')) || curChar == '\n' || curChar == '\t') {
      i++;
      continue;
    }

    // Checks if it is a quote
    if ((curChar == '\\' && input[i+1] == '\"') || curChar == '\"') {
      i = handle_string2(i+1, input, output);
      continue;
    }

    // Checks if it is a space
    if (curChar == ' ') {
      continue;
    }

    buffer[bufferIdx] = curChar;
    bufferIdx++;
  }

  free(input);
  free(buffer);
  return output;
}

// Handles cases that are in quotes
int handle_string2(int i, char *input, vect_t *output) {

  if (input[i] == '\"') {
    i++;
  }

  char *buffer = (char *) malloc(255 * sizeof(char*));
  int bufferIdx = 0;

  while(input[i] != '\"') {
    char curChar = input[i];
    buffer[bufferIdx] = curChar;
    bufferIdx++;
    i++;
  }

  if(buffer[bufferIdx-1] == '\\'){
    buffer[bufferIdx-1] = '\0';
  }
  else{
    buffer[bufferIdx] = '\0';
  }

  vect_add(output, buffer);
  free(buffer);
  return i;
}
