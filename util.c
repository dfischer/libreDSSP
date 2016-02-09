/*	This file is part of libreDSSP.

	Copyright 2016 Alan Beadle

	libreDSSP is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	libreDSSP is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with libreDSSP.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "util.h"
#include "dict.h"
#include "elem.h"

// This can be reduced but it hits every case
int isnum(char * foo){
	int i = 0;
	char temp = foo[0];

	if(foo == NULL) return 0;
	if(strlen(foo) == 0) return 0;

	// If first is minus, scan from beginning to see if all are digits
	if((temp=='-') && (strlen(foo) > 1)){
		for (i=1; i < (strlen(foo)); i++){
			if(!isdigit(foo[i])) return 0;
		}
	}else for (i=0; i < (strlen(foo)); i++){
		if(!isdigit(foo[i])) return 0;
	}
	return 1;
}


// Takes a linked command sequence and attempts to run it as DSSP code
elem * run(elem * stack, elem * seqHead, dict * vocab){
	elem * seqPrev; // We use this to free each element after we are done reading it
	elem * tempStack;
	elem * tempSeq;

	tempSeq = seqHead;
	do{
		assert(tempSeq != NULL);
		if(isnum(tempSeq->chars)){ // Numerical constant
			tempStack = stack;
			stack = malloc(sizeof(elem));
			stack->next = tempStack;
			stack->value = atoi(tempSeq->chars);
		}else if (!strcmp(tempSeq->chars, ":")){ // Function declaration
			tempSeq = defWord(seqHead,vocab);
		}else if (tempSeq->chars[0] == '['){ // Comment
			// Do nothing
		}else{ // Not a number or a function declaration
			stack = wordRun(tempSeq, stack, vocab);
		}
		if(tempSeq != NULL){ // This will be NULL if we did a function declaration
			if(tempSeq->next == NULL) return stack; // Fixes DO loop crashes
			seqPrev = tempSeq;
			tempSeq = tempSeq->next;
			free(seqPrev); // We should be done with this element
		}
	}while(tempSeq != NULL);
	return stack;
}

// Takes command line and splits it by spaces, returns sequence
elem * parseInput(char * line){
	char ch;
	int i = 0;
	int j = 0;
	elem * seqHead;
	elem * seqcurr;
	elem * seqtail;

	seqHead = malloc(sizeof(elem));
	seqtail = seqHead;
	seqtail->next = NULL;

	// TODO Not safe!
	while((line[j] != '\0') && (j < 79)){
		ch = line[j++];
		if (ch == '[') {
			seqtail->chars[i++] = '[';
			while(((ch = line[j++]) != ']')){
				if(i>8) break; // TODO This limits a word or comment to 8 chars due to fixed size in struct
				seqtail->chars[i++] = ch;
			}
			seqtail->chars[i++] = ']';

		} else if(ch != ' ') {
			if(i>8) break; // TODO This limits a word or comment to 8 chars due to fixed size in struct
			seqtail->chars[i++] = ch;

		} else if ((ch == ' ') && (i != 0)) { // Handles adjacent spaces
			seqtail->chars[i] = '\0';
			seqcurr = seqtail;
			seqtail = malloc(sizeof(elem));
			seqcurr->next = seqtail; // old tail used to point to NULL, now to new elem
			seqtail->next = NULL;
			i=0;
		}
	}
	seqtail->chars[i] =  '\0';
	return seqHead;
}

char * prompt(){
	int i = 0;
	char * line = malloc(80 * sizeof(char)); // TODO: This limits line size to 80!
	char ch;
	printf("* ");
	while(((ch = getchar()) != '\n') && (i < 79)){
		line[i++] = ch;
	}
	line[i] = '\0';
	return line;
}

