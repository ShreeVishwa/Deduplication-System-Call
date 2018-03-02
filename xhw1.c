#include <asm/unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include "params.h"
#include "flags.h"
#ifndef __NR_xdedup
#error xdedup system call not defined
#endif

//struct Param {
//	char* fname;
//};

int main(int argc, char* const argv[])
{
	int ch;
	struct Param *params = (struct Param*)malloc(sizeof(struct Param));
	params->b1 = 0x00;
	//printf("filename 1: %s, %s\n",params->f1name, argv[1]);
	//printf("filename 2: %s, %s\n",params->f2name, argv[2]);
	//printf("Outfilename: %s, %s\n",params->outfile,argv[3]);
	//int len;
	params->f1name = (char*)malloc(sizeof(char)*PATH_MAX);
	params->f2name = (char*)malloc(sizeof(char)*PATH_MAX);
	params->outfile = (char*)malloc(sizeof(char)*PATH_MAX);
	int rc;
		
	while ((ch = getopt(argc, argv, "npd")) != -1) {
		switch (ch) {
		case 'n':
			params->b1 = params->b1 | FLAG_N;
			//printf("I am in N\n"); 
			break;
		case 'p':
			params->b1 = params->b1 | FLAG_P;
			//printf("I am in P\n");
			break;
		case 'd':
			params->b1 = params->b1 | FLAG_D;
			//printf("I am un D\n");
			break;
		default:
			printf("No arguments have been entered");
			break;
		}
	}
	//printf("%u\n",params->b1);

	if(argv[optind] == NULL){
		printf("Insufficient arguments\n");
		return -1;
	}
	else{
		strcpy(params->f1name,argv[optind++]);
	}

	if(argv[optind] == NULL){
		printf("Insufficient arguments\n");
		return -1;
	}
	else{
		strcpy(params->f2name,argv[optind++]);
	}

	if((params->b1 == 2 || params->b1 == 6)){
 		if(argv[optind] == NULL){
			printf("Insufficient arguments\n");
			return -1;
		}
		else{	
			strcpy(params->outfile,argv[optind++]);
		}
	}

	//printf("Passed all the cases\n");	
	//
	if(access(params->f1name,F_OK) == -1){
		printf("File 1 does not exist\n");
		return -1;
	}

	if(access(params->f1name,R_OK) == -1){
		printf("File 1 doesn't have enough permissions\n");
		return -EPERM;
	}
	
	if(access(params->f2name,F_OK) == -1){
		printf("File 2 doesn't exist\n");
		return -1;
	}

	if(access(params->f2name,R_OK) == -1){
		printf("File 2 doesn't have enough permissions\n");
		return -1;
	}

	
	//printf("filename 1: %s\n",params->f1name);
	//printf("filename 2: %s\n",params->f2name);
	//printf("outfilename: %s\n",params->outfile);
	
	
  	rc = syscall(__NR_xdedup, (void*)params);
	printf("%d\n",rc);
	if (rc == 0)
		printf("Number of bytes is  %d\n", rc);
	else
		printf("syscall returned %d (errno=%d)\n", rc, errno);

	exit(rc);
	
}
