#include "VMsimulator.h"

/*
 * Justin Barish
 * Jack Kraszewski
 *
 * We pledge our honor that we have abided by the Stevens Honor System 
 */

/*store information about a page */
struct page {
	int pageNumber; /*unique page number */
	int valid; /*whether page is in memory*/
	unsigned long accessed; /*time accessed */
};
typedef struct page* Page;

/*store information for each process */
struct process {
	int pID; /*id of the process */
	int totalMem; /*memory the process needs */
};
typedef struct process* Process;

typedef enum {FIFO, LRU, CLOCK} replacementAlgoType; /*data type for replacement algo type */

replacementAlgoType replacementAlgo;
int prePage; /*1 if use prepage, 0 if not, given as arg */
int pageSize; /*size of pages, given as arg */

int numProcs=0; /*number of proccess in process list */

Page** pageTableList; /*List of page tables*/

Process* procs; /*List of processes */

/*read in the processes from the process list */
void readProcs(char* fileName){
	FILE* fp;
	char str[25]; /*max line size is 25 (10 for first int, 1 for space, 10 for second int, 1 for \0) */
	int procNum; /*temp storage of a procs id num */
	int memSize; /*temp storage of a procs total memory */
	fp = fopen(fileName, "r"); /*open the file */

	/*list of processes */
	procs = (Process*)malloc(sizeof(Process)*1);
	
	/*read in from file until last line */
	while(fgets(str, 25, fp)!=NULL){
		if(strlen(str)>1){ /*ensure no blank lines at end of file are stored */
			sscanf(str, "%i %i",&procNum, &memSize); /*format input string into temp variables */
			Process p = (Process)malloc(sizeof(struct process)); /*allocate a new process */
			p->pID = procNum;
			p->totalMem = memSize;
			procs[numProcs] = p; /*add the new process to the list */
			
			/*increase the size of the process array*/
			numProcs++;
			procs = (Process*)realloc(procs, +sizeof(Process)*numProcs+1);
		}
		
	}
	fclose(fp);
}


int main(int argc, char *argv[]){
	
	/*extract args and error check */
	if(argc < 6){
		perror("Must provide 5 arguments: plist and ptrace files, page size, replacement algo, and prepaging");
		exit(1);
	}
	
	if(strcmp(argv[5], "+")==0){
		prePage = 1;
	}else if (strcmp(argv[5], "-")==0){
		prePage = 0;
	}else{
		perror("Must give a + or - as prepage argument");
		exit(1);
	} 

	if(strcmp(argv[4], "FIFO")==0){
		replacementAlgo = FIFO;
	}else if (strcmp(argv[4], "LRU")==0){
		replacementAlgo = LRU;
	}else if (strcmp(argv[4], "Clock")==0 || strcmp(argv[4], "clock")==0){
		replacementAlgo = CLOCK;
	}else{
		perror("Must give Clock, LRU, or FIFO as replacement algorithm argument");
		exit(1);	
	} 
	
	if(atoi(argv[3]) >0){
		pageSize = atoi(argv[3]);
	}else{
		perror("Must give a positive, non-zero page size ");
		exit(1);	
	}
	
	/*read in procs from file */
	readProcs(argv[1]);
	/* printf("numProcs:%i\n", numProcs);
	for(int i = 0; i<numProcs; i++){
		printf("i=%i, Proc: %i, Mem: %i\n", i,procs[i]->pID, procs[i]->totalMem);
	} */
	
	
}