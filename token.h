#ifndef _TOKEN_H
#define _TOKEN_H

// Parses stdin to creat a vector of tokens
vect_t *parseInput(char *input);

// Handles when a string is hit while parsing
int handle_string(int i, char *input, vect_t *output);

int isSpecialChar(char c);
#endif /* ifndef _TOKEN_H */
