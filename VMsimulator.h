#ifndef VMSIMULATOR_H
#define VMSIMULATOR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


struct frameInfo{
	int proc;
	int memLoc;
 };
typedef struct frameInfo* frame;
frame dequeueFirst();
void enqueue(frame prod);
int getNumElements();
#endif