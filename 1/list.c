#include "list.h"

#include <stdlib.h>
#include <string.h>

void listInit(List** L, size_t data_size, func_ptr free_func) {
	(*L) = (List*)malloc(sizeof(List));

	(*L)->head = (*L)->tail = NULL;

	(*L)->data_size = data_size;
	(*L)->free_func = free_func;
}

void listPushBack(List* L, void* data) {
	ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
	new_node->data = malloc(L->data_size);

	memcpy(new_node->data, data, L->data_size);
	new_node->next = new_node->prev = NULL;

	if (listIsEmpty(L))
		L->head = L->tail = new_node;
	else {
		new_node->prev = L->tail;
		L->tail->next = new_node;
		L->tail = new_node;
	}
}

void listPushFront(List* L, void* data) {
	ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));

	memcpy(new_node->data, data, L->data_size);
	new_node->next = new_node->prev = NULL;


	if (listIsEmpty(L))
		L->head = L->tail = new_node;
	else {
		L->head->prev = new_node;
		new_node->next = L->head;
		L->head = new_node;
	}
}

void listPopBack(List* L) {
	if (!listIsEmpty(L)) {
		if (L->free_func)
			L->free_func(L->tail->data);

		ListNode* new_tail = L->tail->prev;

		free(L->tail);

		L->tail = new_tail;
	}
}

void listPopFront(List* L) {
	if (!listIsEmpty(L)) {
		if (L->free_func)
			L->free_func(L->head->data);

		ListNode* new_head = L->head->next;

		free(L->head);

		L->head = new_head;
	}
}

void* listBack(const List* L) {
	return listIsEmpty(L) ? NULL : L->tail->data;
}

void* listFront(const List* L) {
	return listIsEmpty(L) ? NULL : L->head->data;
}

void listForEach(List* L, func_ptr proces_func) {
	ListNode* iter = L->head;

	do {
		proces_func(iter->data);

		iter = iter->next;
	} while (iter);
}

void listDeleteNode(List* L, ListNode* node) {
	if (!listIsEmpty(L)) {
		if (node != L->head)
			node->prev->next = node->next;
		else
			L->head = L->tail = NULL;

		if (L->free_func)
			L->free_func(node->data);

		free(node);
	}
}

void listClear(List* L) {
	if (!listIsEmpty(L)) {
		ListNode* iter = L->head;

		do {
			ListNode* delete_node = iter;

			iter = iter->next;

			if (L->free_func)
				L->free_func(iter->data);

			free(delete_node);
		} while (iter);

		L->head = L->tail = NULL;
	}
}

void listFree(List** L) {
	listClear(*L);
	free(*L);

	*L = NULL;
}
