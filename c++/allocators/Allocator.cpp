//============================================================================
// Name        : Allocator.cpp
// Author      : Shtrikh17
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

struct chunk{
	char* start;
	unsigned int length;
	struct chunk* prev;
	struct chunk* next;
};

class SmallAllocator {
private:
        char Memory[1048576];
        struct chunk* first;
        struct chunk* last;
public:
        SmallAllocator(): first(NULL), last(NULL){};
        void *Alloc(unsigned int Size);
        void *ReAlloc(void *Pointer, unsigned int Size);
        void Free(void *Pointer);
};

void* SmallAllocator::Alloc(unsigned int Size){
	if(!first){
		struct chunk* p = new struct chunk;
		p->start = Memory;
		p->length = Size;
		p->prev = NULL;
		p->next = NULL;

		first = p;
		last = p;
		return p->start;
	}
	else{
		struct chunk* p = new struct chunk;
		p->start = last->start + last->length;
		p->length = Size;
		p->prev = last;
		p->next = NULL;
		last->next = p;
		last = p;
		return p->start;
	}
}

void* SmallAllocator::ReAlloc(void* Pointer, unsigned int Size){
	struct chunk* tmp = first;
	char* ptr = (char*)Pointer;
	while(tmp){
		if(tmp->start==ptr) break;
		tmp = tmp->next;
	}
	if(!tmp) return Pointer;
	char* p = (char*)Alloc(Size);
	for(unsigned int i=0; i<tmp->length; i++){
		*(p+i) = *(tmp->start+i);
	}
	Free(tmp->start);
	return p;
}

void SmallAllocator::Free(void* Pointer){
	if(first==last){
		if(first==NULL) return;
		first = NULL;
		last = NULL;
	}
	else{
		struct chunk* tmp = first;
		char* p = (char*) Pointer;
		while(tmp){
			if(tmp->start==Pointer) break;
			tmp = tmp->next;
		}
		if(tmp==NULL) return;
		if(tmp->next) tmp->next->prev = tmp->prev;
		if(tmp->prev) tmp->prev->next = tmp->next;
		if(tmp==first) first == tmp->next;
		if(tmp==last) last = tmp->prev;
	}
}


int main() {

	SmallAllocator A1;
	int * A1_P1 = (int *) A1.Alloc(sizeof(int));
	A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
	A1.Free(A1_P1);

	SmallAllocator A2;
	int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;

	int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;

	A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
	for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;

	A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
	for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
	A2.Free(A2_P1);
	A2.Free(A2_P2);

	return 0;
}
