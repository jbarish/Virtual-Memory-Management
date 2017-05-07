This program simulates a virtual memory management system.   
It implements three different page replacement algorithms:   
   First-in, First-out,
   Least Recently Used,
   Clock-based Replacement
 It also implements both demand and prepaging methods.
 The page size is additionally of variable size. 
 
 The program, upon finishing executing, displays the number of page faults that occured.
 
The program takes in the following arguments:      
  P1) Text file for the list of processes   
  P2) Text file for the trace of execution of the processes   
  P3) Size of the pages (powers of 2 between 1 and 32)   
  P4) Page replacement algorithm (FIFO, LRU, or Clock)   
  P5) Whether to use demand or pre-paging (+ for pre-paging, - for demand)   
  
Sample execution of the program:
> ./VMsimulator plist.txt ptrace.txt 2 FIFO +
