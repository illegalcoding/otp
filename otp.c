// includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifndef __OpenBSD__
#include <sodium.h>
#include <sodium/randombytes.h>
#endif
// definitions, global variables
#define NUMSIZE 240
const char * alphabet = "abcdefghijklmnopqrstuvwxyz";
// function protypes
int keygen(uint32_t keys[NUMSIZE], char* filename);
int write_keys(uint32_t keys[NUMSIZE], char* filename);
int encrypt_message(char* input_message, uint32_t output_message[NUMSIZE/2], char* filename);
int ingest_keys(char* filename, uint32_t keys[NUMSIZE/2]);
char* get_message(char* message);
int get_abcpos(char inputChar);
// main function
int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "%s\n", "Not enough args!");
		return -1;
	}
	// variables
	uint32_t keys[NUMSIZE] = {0};
	char* message;
	uint32_t encrypted_message[NUMSIZE] = {0};
	size_t message_length;
	int encrypted_length;
	char * filename = argv[1];
	char response;
	bool doEncrypt = false;
	bool doKeygen = false;
	// main stuff
    message = (char *)malloc(NUMSIZE * sizeof(char));
    //encrypted_message = (char *)malloc(NUMSIZE * sizeof(char));
	printf("%s\n","?");
	scanf(" %c",&response);
	response = tolower(response);
	if(response == 'e') {
		doEncrypt = true;
	} else if(response == 'k') {
		doKeygen = true;
	} else {
		fprintf(stderr, "Invalid response!");
		free(message);
		return -1;
	}
	if(doKeygen) {
		keygen(keys, filename);
		write_keys(keys,filename);
		free(message);
		return 0;
	} else if(doEncrypt) {
		get_message(message);
		message_length = strlen(message);
		if(message_length > NUMSIZE/2) {
			fprintf(stderr,"%s\n","Message too long!");
			return -1;
		}
		encrypted_length = encrypt_message(message, encrypted_message, filename);
		if(encrypted_length != -1) {
			for(int i = 0; i < encrypted_length; i++) {
				printf("%d",encrypted_message[i]);	
				printf("%s", " ");
			}
		} else {
			free(message);
			return -1;
		}
		printf("\n");
		free(message);
		return 0;
	}
	free(message);
	return -1;
}
// other functions
int keygen(uint32_t keys[NUMSIZE], char* filename){
	for(int i = 0;i<NUMSIZE; i++) {
		#ifndef __OpenBSD__
		keys[i] = randombytes_uniform(10);
		#else
		keys[i] = arc4random_uniform(10);
		#endif
	}
	
	return 0;
}

char* get_message(char* message) {
	printf("%s", "Enter message: ");
	scanf("%s",message);
	
	return message;
}

int encrypt_message(char* input_message, uint32_t output_message[NUMSIZE/2], char* filename) {
	// variables
	uint32_t keys[NUMSIZE/2];
	size_t length;
	length = strlen(input_message);
	int abcpos = 0;
	// ingest keys
	ingest_keys(filename, keys);
	// convert message to lowercase
	for(int i = 0; i < length; i++) {
		input_message[i] = tolower(input_message[i]);
	}
	// encrypt message
	for(int i = 0; i < length; i++) {
		char c = input_message[i];
		int keydigit = 0;
		int encrypted_char;
		if((abcpos = get_abcpos(c)) == -1) {
			return -1;	
		}
		keydigit = keys[i];
		encrypted_char = abcpos + keydigit;
		output_message[i] = encrypted_char;	
	}
	return length;	
}

int get_abcpos(char inputChar) {
	int pos;
	const char* found;
	found = strchr(alphabet,inputChar);
	pos = found - alphabet;
	if(!found) {
		fprintf(stderr,"%s\n","Illegal character(s)");	
		return -1;
	} else if(pos == 26) {
		pos = 0;
	}
	return pos;
}
int write_keys(uint32_t keys[NUMSIZE], char* filename) {
	FILE* fp;
	fp = fopen(filename, "w");
	int counter = 0;
	for(int i = 0; i < NUMSIZE-4; i+=5) {
		fprintf(fp,"%d",keys[i]);
		fprintf(fp,"%d",keys[i+1]);
		fprintf(fp,"%d",keys[i+2]);
		fprintf(fp,"%d",keys[i+3]);
		fprintf(fp,"%d",keys[i+4]);
		fprintf(fp,"%s", " ");
		counter++;
		if((counter % 6) == 0) {
			fprintf(fp,"%s","\n");
		}

	}
	fclose(fp);
	return 0;
}
int ingest_keys(char* filename, uint32_t keys[NUMSIZE/2]) {
	uint32_t debugkeys[NUMSIZE/2] = {0};
	FILE* fp;
	char* stringbuf;
	char* intermediatebuf;
	char intermediatestring[3] = {0};
	size_t length;
	int loadcounter = 0;
	fp = fopen(filename, "r");
	fseek(fp,0,SEEK_END);
	length = ftell(fp);
	fseek(fp,0,SEEK_SET);
	stringbuf = malloc(length+1);
	intermediatebuf = malloc(length+1);
	explicit_bzero(stringbuf, length);
	explicit_bzero(intermediatebuf, length);
	fread(stringbuf,1,length,fp);
	fclose(fp);

	// clean up buffers
	int ret = 0;
	for(int i = 0; i < strlen(stringbuf); i++) {
		ret = isalnum(stringbuf[i]);
		if(ret >= 1) {
			intermediatebuf[strlen(intermediatebuf)] = stringbuf[i];
		}
		// printf("%d\n",ret); // debug only
	}
	// write two digit numbers into keys
	for(int i = 1; i < NUMSIZE; i+=2) {
		// create string out of intermediatebuf[i-1] + intermediatebuf[i] then atoi that into keys[i]
		sprintf(intermediatestring,"%c%c",intermediatebuf[i-1],intermediatebuf[i]);
		keys[loadcounter] = atoi(intermediatestring);
		loadcounter++;
	}
	// printf("%s", intermediatebuf); // debug only
	// TODO: return amount of numbers;
	free(stringbuf);
	free(intermediatebuf);
	return 0;
}
