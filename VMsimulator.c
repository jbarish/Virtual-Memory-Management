#include "VMsimulator.h"

#define MEMORY_SIZE 512

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
};typedef struct page* Page;

/*store information for each process */
struct process {
	int pID; /*id of the process */
	int totalMem; /*memory the process needs */
};typedef struct process* Process;

typedef enum {FIFO, LRU, CLOCK} replacementAlgoType; /*data type for replacement algo type */

replacementAlgoType replacementAlgo; /*given as arg*/
int prePage; /*1 if use prepage, 0 if not, given as arg */
int pageSize; /*size of pages, given as arg */

int numProcs=0; /*umber of proccess in process list */

Page** pageTableList; /*List of page tables*/
Process* procs; /*List of processes */
int* memory;
int pageCounter = 0;

int numProcsTwo = 0; /* number of processes in ptrace file */

frame* frames; /* List of frames */

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

/* method to read in the process requests from the ptrace file */
void readFrames(char* fileName){
  /*implement*/
}

/*load initial set into memory*/
void defaultLoad(){
	int framesPerProc = (MEMORY_SIZE/pageSize)/numProcs;
	memory = (int*)malloc(sizeof(MEMORY_SIZE/pageSize));
	
	/*initalize all memory frames to -1 */
	for(int i = 0; i<MEMORY_SIZE/pageSize; i++){
		memory[i] = -1;
	}
	
	/*add pages to memory, and set valid bit */
	for(int i = 0; i<numProcs; i++){
		for(int j=0; j<framesPerProc && j<procs[i]->totalMem/pageSize; j++){
			memory[i*framesPerProc+j] =  ((pageTableList[i])[j])->pageNumber;
			((pageTableList[i])[j])->valid = 1;
		}
	}
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
	
	/*make the pageTable List */
	pageTableList = (Page**)malloc(sizeof(Page*)*numProcs);
	for(int i = 0; i<numProcs; i++){
		pageTableList[i] = (Page*)malloc(sizeof(Page)*procs[i]->totalMem/pageSize);
		for(int j = 0; j<procs[i]->totalMem/pageSize; j++){
			(pageTableList[i])[j] = (Page)malloc(sizeof(struct page));
			(pageTableList[i])[j]->pageNumber = pageCounter++;
			(pageTableList[i])[j]->valid = 0;
			(pageTableList[i])[j]->accessed = 0;
		}
	}
	
	/*load resident set*/
	defaultLoad();
	
	for(int i = 0; i<MEMORY_SIZE/pageSize; i++){
		printf("%i,", memory[i]);
	}
	printf("\n");
}

