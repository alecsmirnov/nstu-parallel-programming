#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef void (*func_ptr)(void*);

typedef struct ListNode {
	void* data;

	struct ListNode* next;
	struct ListNode* prev;
} ListNode, Node;

typedef struct List {
	struct ListNode* head;
	struct ListNode* tail;

	size_t data_size;

	func_ptr free_func;
} List;

static inline bool listIsEmpty(const List* L) {
	return L->head == NULL;
}

void listInit(List** L, size_t data_size, func_ptr free_func);

void listPushBack(List* L, void* data);
void listPushFront(List* L, void* data);

void listPopBack(List* L);
void listPopFront(List* L);

void* listBack(const List* L);
void* listFront(const List* L);

void listForEach(List* L, func_ptr proces_func);

void listDeleteNode(List* L, ListNode* node);

void listClear(List* L);
void listFree(List** L);

#endif
