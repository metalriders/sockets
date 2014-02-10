#include <stdio.h> 
#include "DLinkedList.h"

int main(){
	List * list;
	list = newList();
	addNode(list, (void*) 3);
	addNode(list, (void*)4);
	addNode(list, (void*)6);
	printList(list);
	deleteList(list);
	printList(list);
	return 0;
}
