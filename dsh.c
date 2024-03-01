/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 * 
 * 	Modified on: Feb 17 2024
 * 		Author: tyler
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_TOKENS 10
#define MAX_TOKEN_LENGTH 50

/**
 * Print the current working directory.
 */
void pwd(){
    char *cwd; // current working directory
    size_t size = pathconf(".", _PC_PATH_MAX); // max 

    if ((cwd = (char *)malloc(size)) != NULL) { // wanted to try using malloc here
        if (getcwd(cwd, size) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        free(cwd);  // Free allocated memory
    } else {
        perror("dsh");
    }
}
/**
 * Change current directory
 */
void cd(char *path) {
    if(path == NULL || *path == '\0'){
        char *home = getenv("HOME");
        chdir(home);
    }
    else if(chdir(path) != 0){
        perror(path);
    }
}

/**
 * Trim preceding and trailing whitespaces of a string
 * 
 * @param str The string to be trimmed
 */
void editWhitespace(char *str) {
    // preceding whitespaces
    int start = 0;
    while (isspace(str[start])) {
        start++;
    }

    // trailing whitespaces
    int end = strlen(str) - 1;
    while (end > start && isspace(str[end])) {
        end--;
    }

    // Shift the string
    int i = 0;
    int j = 0;
    for (i = start; i <= end; i++) {
        str[j++] = str[i];
    }

    // Null-terminate the trimmed string
    str[j] = '\0';
}

/**
 * Split a string into tokens based on delimiter
 *
 * @param str The string to be split
 * @param delim Delimiter used to split the string
 * @param numTokens Output for the number of tokens
 * @return Array of strings representing the tokens
 */
char** split(char *str, const char *delim, int *numTokens) {
    // Allocate memory for array of pointers
    char **tokens = (char **)malloc(MAX_TOKENS * sizeof(char *));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for each token
    int i;
    for (i = 0; i < MAX_TOKENS; i++) {
        tokens[i] = (char *)malloc(MAX_TOKEN_LENGTH * sizeof(char));
        if (!tokens[i]) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }

    // split the input string into tokens
    char *token = strtok(str, delim);
    i = 0;
    while (token != NULL && i < MAX_TOKENS) {
        // Copy each token into array element
        strcpy(tokens[i++], token);
        token = strtok(NULL, delim);
    }
    *numTokens = i;

    // Return the tokens
    return tokens;
}


/**
 * Free memory allocated for an array of tokens
 *
 * @param tokens The array of tokens to be freed
 */
void freeTokens(char **tokens) {
    // Loop through everthing and free it
    for (int i = 0; i < MAX_TOKENS; i++) {
        free(tokens[i]);
    }
    free(tokens);
}



/**
 * Mode 1 by entering the full path of whatever you want, you can run it
 * 
 * @param cmdline The command line entered by the user
 */
void mode1(char *cmdline) {
    // Does file exist?
    if(access(cmdline, F_OK | X_OK) == 0){
        pid_t pid = fork();

        if (pid == 0){              // child process
            char *arg[] = {cmdline, NULL};
            execv(cmdline, arg);
            perror(cmdline);        // print error if failed
            exit(1);
        } else {                    // parent proces
            wait(NULL);             // wait for child to finish
        }
    } else{                           // File doesn't exist or not executable
        fprintf(stderr, "Error: File not found or not executable\n");
    }
}

/**
 * mode 2 the not full path of running things
 * 
 * Unfortunately mode 2 doesn't work very well, for example while I can run ls,
 * ls -l does not work
 * 
 *  * @param cmdline The command line entered by the user
 */
void mode2(char *cmdline) {
    char *env = getenv("PATH");
    char *token = strtok(env, ":");

    while (token != NULL) {
        // construct path
        char path[MAXBUF];
        snprintf(path, sizeof(path), "%s/%s", token, cmdline);

        // check if file exists and executable
        if (access(path, F_OK | X_OK) == 0) {
            int numTokens;
            char **tokens = split(cmdline, " ", &numTokens);

            pid_t pid = fork(); // call fork

            if (pid == 0) { // child process
                char *arg[MAX_TOKENS];
                for (int i = 0; i < numTokens; i++) {
                    arg[i] = tokens[i];
                }
                arg[numTokens] = NULL;

                execv(path, arg);
                perror("execv"); // print error if fail
                exit(1);
            } else { // Parent process
                wait(NULL); // Wait for child to finish

                // Free the allocated memory for tokens
                freeTokens(tokens);
                return; // return when successful
            }
        }

        token = strtok(NULL, ":");
    }

    // print error if command wasn't found after looping
    fprintf(stderr, "Error: Command not found\n");
}



/**
 * Main function to process user commands in the dsh
 *
 * @param cmdline The command line entered by the user
 */
void dShell(char *cmdline) {
    // get rid of leading and trailing whitespaces
    editWhitespace(cmdline);

    // If the input is empty, do nothing
    if (strlen(cmdline) == 0) {
        return;
    }

    // Built-in commands
    if (strcmp(cmdline, "exit") == 0) {
        free(cmdline);
        exit(0);
    } else if (strcmp(cmdline, "pwd") == 0) {
        pwd();
    } else if (strncmp(cmdline, "cd", 2) == 0) {
        char *path = cmdline + 2;   // get path
		editWhitespace(path);       // get rid of space
        cd(path);                   // use cd
    // Mode 1
    } else if (cmdline[0] == '/') {
        mode1(cmdline);
    // Mode 2
    } else {
        mode2(cmdline);
    }
}