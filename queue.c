#include "VMsimulator.h"

/*
 * Justin Barish
 * Jack Kraszewski
 *
 * We pledge our honor that we have abided by the Stevens Honor System 
 */

struct node{
	frame p;
	struct node* next;
	struct node* prev;
};
typedef struct node* Node;
Node head = NULL;
Node tail = NULL;

/*
 *  [tail]                   [head]
 *     |                       |
 *  |frame    |   |frame  |   |frame    |
 *  |prev=null|<--|=prev  |<--|=prev    |
 *  |next = --|-->|next=--|-->|next=null|
 *
 * Add to tail, remove from head with dequeueFirst
 */
 
/*num of elements currently in queue*/
int numElements = 0;


/*remove item at head
 *returns NULL if nothing in list
 */
frame dequeueFirst(){
	if(head!=NULL){
		frame pTemp = head->p;
		Node temp = head;
		if(head->prev!=NULL){
			temp->prev->next=NULL;
		}
		head=temp->prev;

		free(temp);
		numElements--;
		if(numElements==0){
			tail ==NULL;
		}
		return pTemp;
	}
	return NULL;
}

/*add to tail*/
void enqueue(frame prod){
	if(head!=NULL){
		Node new = (Node)malloc(sizeof(struct node));
		new->p = prod;
		tail->prev = new;
		new->next = tail;
		tail = new;
		new->prev = NULL;
	}else{
		/*first thing in list*/
		Node new = (Node)malloc(sizeof(struct node));
		new->p = prod;
		new->prev = NULL;
		new->next=NULL;
		head = new;
		tail = new;
	}
	numElements++;
}

int getNumElements(){
	return numElements;
}
