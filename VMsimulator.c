#include "VMsimulator.h"
//#define TEST /*will put in debug mode, printing out all steps */
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
	unsigned long added;
	int r;
};typedef struct page* Page;

struct memoryEntry {
  int id; /* id for each entry */
  int proc; /* which proc it comes from */
};typedef struct memoryEntry* MemEntry;

/*store information for each process */
struct process {
	int pID; /*id of the process */
	int totalMem; /*memory the process needs */
};typedef struct process* Process;

/*store information about each proccess's clock hand state */
struct clockInfo{
	int minMemSlot;
	int maxMemSlot;
	int currMemLoc;
};typedef struct clockInfo* ClockInfo;

typedef enum {FIFO, LRU, CLOCK} replacementAlgoType; /*data type for replacement algo type */

replacementAlgoType replacementAlgo; /*given as arg*/
int prePage; 	/*1 if use prepage, 0 if not, given as arg */
int pageSize; 	/*size of pages, given as arg */

int numProcs=0; /*umber of proccess in process list */

Page** pageTableList; 	/*List of page tables*/
Process* procs; 		/*List of processes */
MemEntry* memory; 		/*Our Main Memory */
ClockInfo* clockList; 	/*list of process's clocks for clock*/

int pageCounter = 0; 	/* used for creating page ids */
int numProcsTwo = 0; 	/* number of processes in ptrace file */

unsigned long counter=0; 			/* running counter as 'clock' ticks */
int memorySlots = MEMORY_SIZE; 		/* number of slots in memory */
unsigned long pageFaultCounter=0; 	/*number of page faults that occur */


/*forward declarations*/
void readProcs(char* fileName);
void readFrames(char* fileName);
MemEntry makeMemoryEntry(int id, int proc);
void defaultLoad();
int inMemory(int id);
int repFIFO(MemEntry id);
void pageReplacement();
void printMemory();


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
  FILE* fp;
  char str[25]; /* max line size is 25 (10 for first int, 1 for space, 10 for second int, 1 for \0) */
  int procNum; /* temp storage of a proc id num*/
  int memSize; /* temp storage of a procs total memory */
  fp = fopen(fileName, "r"); /* open the file */

  /* read in from file until last line */
  while(fgets(str, 25, fp) != NULL){
    if(strlen(str) > 1){ /* ensure no blank lines at end of file are stored */
      sscanf(str, "%i %i" ,&procNum, &memSize); /* format input string into temp variables */
      
      /* create frame */
      frame f = (frame)malloc(sizeof(struct frameInfo)); /* allocates for a new frame */
      f->proc = procNum;
      f->memLoc = memSize;

      /* adds frame to queue */
      enqueue(f);
    }
  }
  fclose(fp);
}

/*Makes a memory entry to add to memory
 *Also, for the page to be added in memory, sets its counter, r, valid, and accessed bits
 */
MemEntry makeMemoryEntry(int id, int proc){
  MemEntry mem = malloc(sizeof(struct memoryEntry));
  mem->id = id;
  mem->proc = proc;
  Page temp = ((pageTableList[proc])[id-((pageTableList[proc])[0])->pageNumber]);
  temp->valid = 1;
  temp->added = counter;
  temp->accessed = counter;
  temp->r = 1;
  return mem;
}

/*load initial set into memory*/
void defaultLoad(){
	
	
	memorySlots = MEMORY_SIZE/pageSize;
	int framesPerProc = memorySlots/numProcs;
	
	memory = (MemEntry*)malloc(sizeof(MemEntry) * memorySlots);
	clockList = malloc(sizeof(ClockInfo)*numProcs);
	/*initalize all memory frames to -1 */
	for(int i = 0; i<memorySlots; i++){
	  memory[i] = NULL;
	}
	int i;
	int j=0;
	/*add pages to memory, and set valid bit */
	for( i= 0; i<numProcs; i++){
	  j=0;
	  clockList[i]= malloc(sizeof(struct clockInfo));
	  clockList[i]->minMemSlot = i*framesPerProc+j;
	  
	  for(j=0; j<framesPerProc && j<(int)ceil(procs[i]->totalMem/(double)pageSize); j++){
	    memory[i*framesPerProc+j] = makeMemoryEntry(((pageTableList[i])[j])->pageNumber, i);
	  }
	  clockList[i]->maxMemSlot = i*framesPerProc+j-1;
	  clockList[i]->currMemLoc = i*framesPerProc+j-1;
	}
	memorySlots = (i-1)*framesPerProc+j; /*shrink memory if there are extra NULLS at end due to rounding*/
}

/*check if a given page id is in memory*/
int inMemory(int id){
	for(int i = 0; i<memorySlots; i++){
		if(memory[i]->id==id){
			return 1;
		}
	}
	return 0;
}

/*find the next contiguous page for a given page id and proc
 *cheks all pages, and returns -1 if all pages are in memory.
 *Else returns a contiguous page id
 */
int findNextCont(int id, int proc){
  int relId = id - ((pageTableList[proc])[0])->pageNumber;
  int cnt = 0;
  for(int i = relId + 1; i <(int)ceil(procs[proc]->totalMem/(double)pageSize)-1; i++,cnt++){
    if(!inMemory(id+cnt)){
      return id+cnt;
    }
  }
  cnt = 0;
  for(int j = 0; j < relId; j++,cnt++){
    if(!inMemory(cnt)){
      return cnt+((pageTableList[proc])[0])->pageNumber;
    }
  }
  return -1;
}

/*page replacement for LRU, uses accessed bit*/
int repLRU(MemEntry id){
  Page smallestPage = NULL;
  int smallestId = -1;
  unsigned long smallest = ULLONG_MAX;
  for(int i = 0; i<memorySlots; i++){
    Page temp = ((pageTableList[memory[i]->proc])[memory[i]->id-
		((pageTableList[memory[i]->proc])[0])->pageNumber]);
    if(temp->accessed < smallest && memory[i]->proc == id->proc){
      smallest = temp->accessed;
      smallestPage = temp;
      smallestId = i;
    }
  }
  #ifdef TEST
  printf("...Replacing page %i...", memory[smallestId]->id);
  fflush(stdout);
  #endif
  free(memory[smallestId]);
  memory[smallestId] = id;
  smallestPage->valid = 0;
  return 0; 
}

/*page replacement for FIFO, uses added bit*/
int repFIFO(MemEntry id){
  Page smallestPage = NULL;
  int smallestId = -1;
  unsigned long smallest = ULLONG_MAX;
  for(int i = 0; i<memorySlots; i++){
    Page temp = ((pageTableList[memory[i]->proc])[memory[i]->id-
		((pageTableList[memory[i]->proc])[0])->pageNumber]);
    if(temp->added < smallest && memory[i]->proc == id->proc){
      smallest = temp->added;
      smallestPage = temp;
      smallestId = i;
    }
  }
	#ifdef TEST
	printf("...Replacing page %i...", memory[smallestId]->id);
	fflush(stdout);
	#endif
  free(memory[smallestId]);
  memory[smallestId] = id;
  smallestPage->valid = 0;
  return 0;
}

/*page replacement for Clock, uses r bit */
int repClock(MemEntry id){
  ClockInfo procClock = clockList[id->proc];
  while(1){
    if(procClock->currMemLoc < procClock->maxMemSlot){
      procClock->currMemLoc++;
    }
    else{
      procClock->currMemLoc = procClock->minMemSlot;
    }
    Page temp = ((pageTableList[memory[procClock->currMemLoc]->proc])[memory[procClock->currMemLoc]->id-
		((pageTableList[memory[procClock->currMemLoc]->proc])[0])->pageNumber]);
    if(temp->r == 0){
		
		#ifdef TEST
		printf("...Replacing page %i...", memory[procClock->currMemLoc]->id);
		fflush(stdout);
		#endif
		free(memory[procClock->currMemLoc]);
		memory[procClock->currMemLoc] = id;
		temp->valid = 0;
       break;
    }
    if(temp->r == 1){
      temp->r = 0;
    }
  }
  return 0;
}

/*run the program, doing page replacements when neccessary */
void pageReplacement(){
	#ifdef TEST
	printf("Initial Memory:\n");
	printMemory();
	#endif

	/*while there are still frames to process, process them*/
	while(getNumElements()>0){
		counter++;
		frame currFrame = dequeueFirst();
		
		double temp = (double)currFrame->memLoc/(double)pageSize;
		int pageId = (int)ceil(temp) + ((pageTableList[currFrame->proc])[0])->pageNumber -1;
		
		#ifdef TEST
		printf("Frame: Proc %i, Mem %i, Relative mem: %i, Absolute Page: %i  ", 
			currFrame->proc, currFrame->memLoc, (int)ceil(temp), pageId);
		fflush(stdout);
		#endif

		/*if the page is not in memory, perform appropriate page replacement algorithm*/
		if(!inMemory(pageId)){
			pageFaultCounter++;
			#ifdef TEST
			printf("Doing Page Replacement...");
			#endif
			switch(replacementAlgo){
			case FIFO:
			  if(!prePage){
			    repFIFO(makeMemoryEntry(pageId, currFrame->proc));
			  }else{
			    repFIFO(makeMemoryEntry(pageId, currFrame->proc));
			    /*find the next contiguous page, and add it to memory*/
			    int tempPageId = findNextCont(pageId, currFrame->proc);
			    if(tempPageId >= 0){
			      repFIFO(makeMemoryEntry(tempPageId, currFrame->proc));
			    }
			  }
			  break;
			case LRU:
			  if(!prePage){
			    repLRU(makeMemoryEntry(pageId, currFrame->proc));
			  }else{
			    repLRU(makeMemoryEntry(pageId, currFrame->proc));
			    int tempPageId = findNextCont(pageId, currFrame->proc);
			    if(tempPageId >= 0){
			      repLRU(makeMemoryEntry(tempPageId, currFrame->proc));
			    }
			  }
			  break;
			case CLOCK:
			  if(!prePage){
			    repClock(makeMemoryEntry(pageId, currFrame->proc));
			  }else{
			    repClock(makeMemoryEntry(pageId, currFrame->proc));
			    int tempPageId = findNextCont(pageId, currFrame->proc);
			    if(tempPageId >= 0){
			      repClock(makeMemoryEntry(tempPageId, currFrame->proc));
			    }
			  }
			  break;
			}
		}else{
		  /*if page was already in memory, then update its bits*/
			((pageTableList[currFrame->proc])[(int)ceil(temp)-1])->accessed = counter;
			((pageTableList[currFrame->proc])[(int)ceil(temp)-1])->r = 1;
			#ifdef TEST
			printf("...Page already in memory...");
			#endif
		}
		#ifdef TEST
		printf("\n");
		printMemory();
		getchar();
		#endif
	}

}

/*debugging, dump contents of memory*/
void printMemory(){
	for(int i = 0; i<memorySlots; i++){
		//printf("----------------\n");
		printf("PID:%i,", memory[i]->id);
		printf("P:%i,", memory[i]->proc);
		int relID =memory[i]->id-((pageTableList[ memory[i]->proc])[0])->pageNumber;
		Page temp = ((pageTableList[ memory[i]->proc])[relID]);
		printf("RID:%i,", relID);
		printf("V:%i,R:%i,", temp->valid, temp->r);
		printf("AD:%lu,", temp->added);
		printf("AC:%lu,",temp->accessed);	
		if((i+1)%5 ==0){
			printf("\n");
		}else{
			printf("\t| ");
		}
	}
	printf("\n");
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
	readFrames(argv[2]);
	
	/*make the pageTable List */
	pageTableList = (Page**)malloc(sizeof(Page*)*numProcs);
	for(int i = 0; i<numProcs; i++){
		pageTableList[i] = (Page*)malloc(sizeof(Page)*procs[i]->totalMem/pageSize);
		for(int j = 0; j<(ceil(procs[i]->totalMem/(double)pageSize)); j++){
			(pageTableList[i])[j] = (Page)malloc(sizeof(struct page));
			(pageTableList[i])[j]->pageNumber = pageCounter++;
			(pageTableList[i])[j]->valid = 0;
			(pageTableList[i])[j]->accessed = 0;
		}
	}
	
	/*load resident set*/
	defaultLoad();
	pageReplacement();
	printf("Page Faults:%lu\n",pageFaultCounter);
}

