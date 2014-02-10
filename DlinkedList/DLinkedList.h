#ifndef FILE_LIST
#define FILE_LIST

#include <stdlib.h>
#include <stdio.h>

typedef struct NODE{
	void *value;
	struct NODE *prev;
	struct NODE *next;
}Node;

typedef struct LIST{
	int count;
	struct NODE *first;
	struct NODE *last;
}List;

Node *newNode(void *value);
List *newList();
void addNode(List *list, void *value);
void printList(List *list);
void deleteList(List *list);

List *newList(){
	List *new_list;
	new_list = (List*)malloc(sizeof(List));
	new_list->count = 0;
	new_list->first = NULL;
	new_list->last = NULL;
	return new_list;
}

Node *newNode(void *value){
	Node *new_node = (Node*)malloc(sizeof(Node));
	new_node->value = value;
	new_node->prev = NULL;
	new_node->next = NULL;
	return new_node; 
}

void addNode(List *list, void *value){
	if(list == NULL) return;
	
	Node *new_node;
	new_node = newNode(value);
	
	if(list->count == 0){
		list->first = new_node;
		list->last = new_node;
		list->count++;
		return;
	}
	
	Node *last = list->last;
	new_node->prev = last;
	last->next = new_node;
	list->last = new_node;
	list->count++;
}

void deleteList(List *list){
	if(list == NULL) return;
	
	printf("--Deleting List--\n");	
	Node *tmp = list->first;
	Node *tmp2;
		
	while(tmp != NULL){
		tmp2 = tmp;
		tmp = tmp->next;
		tmp2->prev = NULL;
		tmp2->next = NULL;
		tmp2->value = NULL;
		free(tmp2);
	}
	list->first = NULL;
	list->last = NULL;
	free(list);
}

void printList(List *list){
	if(list == NULL) return;
		
	Node *tmp = list->first;
	int count = 0;
	
	printf("--Printing List--\n");
	while(tmp != NULL){
		printf("Node > %i\n", count);
		printf("Address > [%x]\n", tmp);
		printf("Preview Node >  [%x] \n", tmp->prev);
		printf("Next Node > [%x] \n", tmp->next);
		printf("Value > [%i] \n\n", tmp->value);
		tmp = tmp->next;
		count++;
	}
}
#endif
