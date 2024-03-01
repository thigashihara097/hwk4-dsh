/*
 * main.c
 *
 *  Created on: Mar 17 2017
 *      Author: david
 * 
 * 	Modified on: Feb 17 2024
 * 		Author: tyler
 */

	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/resource.h>
	#include <sys/stat.h>
	#include <string.h>
	#include "dsh.h"

	#define MAX_PROC 250


	int main(int argc, char *argv[]) {

		// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
		struct rlimit limit;
		limit.rlim_cur = MAX_PROC;
		limit.rlim_max = MAX_PROC;
		setrlimit(RLIMIT_NPROC, &limit);
		// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //


		char *cmdline = (char*) malloc(MAXBUF); // stores user input from commmand line

		while(1){ 
			printf("dsh> ");

			// reads up to 256 characters into the buffer
			if(fgets(cmdline, MAXBUF, stdin) == NULL){
				exit(0); // Exit the program if EOF is input
			}

			// decided to just put everything in dsh.c
			dShell(cmdline);

		}

		// free memory
		free(cmdline);
		return 0;
	}
