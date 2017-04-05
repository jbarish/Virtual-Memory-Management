#include "VMsimulator.h"

/*
 * Justin Barish
 * Jack Kraszewski
 *
 * We pledge our honor that we have abided by the Stevens Honor System 
 */

struct node{
	int id;
	struct node* next;
	struct node* prev;
};
typedef struct node* Node;

int numElements;
struct max_queue {
	struct node* head;
	struct node* tail;
	int numElements;
	int maxElements;
}; typedef struct max_queue* QueueMax;

/*
 *  [tail]                   [head]
 *     |                       |
 *  |id       |   |id     |   |id       |
 *  |prev=null|<--|=prev  |<--|=prev    |
 *  |next = --|-->|next=--|-->|next=null|
 *
 * Add to head
 */

/*Initialize the QueueMax
 * 
 * Param maxElem  maximum size the QueueMax can grow to
 * Return         the newly malloc'ed QueueMax
 */
QueueMax makeQueue(int maxElem){
	QueueMax q =  (QueueMax)malloc(sizeof(struct max_queue));
	q->maxElements = maxElem;
	q->numElements = 0;
	q->head = NULL;
	return q;
}

/* Handles two operations:
 * If # of elements in QueueMax < max number allowed, then enqueue the new page id.
 *     Returns -1;
 * Otherwise, enqueue new page id, and dequeue oldest page id
 *     Returns id that was dequeued
 * 
 * Param q   the QueueMax
 * Param i   the page id
 * Return    -1 if no page removed, otherwise the page id that was removed
 */
int shove(QueueMax q,int i){
	Node n = (Node)malloc(sizeof(struct node));
	
	n->next = NULL;
	if(q->head == NULL){
	
		/*first thing in list*/
		n->prev = NULL;
		n->id = i;
		q->head = n;
		q->tail = n;
		q->numElements++;
		return -1;
	}else{
		n->prev = q->head;
		n->id = i;
		q->head->next = n;
		q->head = n;
		
		if(q->numElements >= q->maxElements){
			/*remove bottom of queue */
			Node tmp = q->tail;
			q->tail = tmp->next;
			q->tail->prev = NULL;
			int valToReturn = tmp->id;
			free(tmp);
			return valToReturn;
		}else{
			q->numElements++;
			return -1;
		}
	}
}

/* Checks if given id is in the QueueMax
 *
 * Param q  the QueueMax
 * Param i  the id to check for
 * Return   0 if id is not in the QueueMax, 1 otherwise
 */
int check(QueueMax q, int i){
	Node curr = q->head;
	while(curr!=NULL){
		if(curr->id == i){
			return 1;
		}
		curr = curr->prev;
	}
	return 0;
}

/* Will promote a given id to the top of the QueueMax
 * Helper method. Not global 
 *
 * Precondition: the given id is in the QueueMax
 * Param q  the QueueMax
 * Param i  the id to promote
 * 
 */
void promote(QueueMax q, int i){
	Node curr = q->head;
	Node tmp;
	while(curr!=NULL){
		if(curr->id == i){
			tmp = curr;
			break;
		}
		curr = curr->prev;
	}
	if(tmp->prev !=NULL){
		tmp->prev->next = tmp->next;
	}else if (tmp->next!=NULL){
		q->tail = tmp->next;
	}
	if(tmp->next!=NULL){
		tmp->next->prev = tmp->prev;
		tmp->prev = q->head;
		q->head->next = tmp;
		q->head = tmp;
	}
}

/* Will promote a given id to the top of the QueueMax if id is in QueueMax
 * Otherwise will call shove() to add id to the QueueMax.
 *
 * Param q  the QueueMax
 * Param i  the id to promote
 * Return   -1 if nothing removed from QueueMax, otherwise the id of element removed
 */
int promoteShove(QueueMax q, int i){
	if(check(q, i)){
		promote(q, i);
		return -1;
	}else{
		return shove(q,i);
	}
}

/* 
 * Delete all nodes in the QueueMax
 *
 * Param q  the QueueMax
 */
void disposeQueueMax(QueueMax q){
	Node curr = q->head;
	while(curr!=NULL){
		Node tmp = curr->prev;
		free(curr);
		curr = tmp;
	}
}


/*Debugging Only: Prints all elements in the QueueMax */
void printQueueMax(QueueMax q){
	Node curr = q->head;
	while(curr!=NULL){
		printf("%i,", curr->id);
		curr = curr->prev;
	}
	printf("\n");
}

/*Debugging Only */
void test(QueueMax q, int i){
	printf("Returned:%i Size: %i\n ",promoteShove(q,i), q->numElements);
	printQueueMax(q);
}

int main(){
	QueueMax q = makeQueue(5);
	
	test(q,1);
	test(q,2);
	test(q,1);
	test(q,3);
	test(q,4);
	test(q,5);
	test(q,1);
	test(q,6);
	test(q,0);
	test(q,4);
	
	
	printf("%i\n", check(q,1));
	printf("%i\n", check(q,10));
	printf("%i\n", check(q,25));
	printf("%i\n", check(q,21));
	printf("%i\n", check(q,20));	
	disposeQueueMax(q);
		
	return 0;
}

