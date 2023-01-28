#include <inc/lib.h>

// malloc()
//	This function use FIRST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

struct Alloc
{
	uint32 startAddress;
	int framesNum;
};
struct Alloc alloc[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];
int counterOfAlloc=-1;
uint32 start_address=USER_HEAP_START;
struct PossibleAlloc
{
	int framesNum;
	int index;
};
struct PossibleAlloc p_alloc[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE];
int bestIndex=-1;
void* checkMemValid(uint32 numOfPages,int counterOfAlloc)
{
	if(counterOfAlloc>=-1)
	{
		int numOfAllocetedPages=0;
		for(int i=0;i<=counterOfAlloc;i++)
		{
			numOfAllocetedPages+=alloc[i].framesNum;
		}
		int numOFAllPages=(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE;
		if(numOfAllocetedPages+numOfPages>numOFAllPages)
		{
			return NULL;
		}
	}
	return (void*)1;
}

void updateAllocArray(int bestsizeIndex,int counterOfAlloc,int numOfPages,uint32 return_address){
	for(int j=counterOfAlloc;j>bestsizeIndex;j--)
	{
		alloc[j].framesNum=alloc[j-1].framesNum;
		alloc[j].startAddress=alloc[j-1].startAddress;
	}
	int index=bestsizeIndex+1;
	alloc[index].framesNum=0;
	alloc[index].startAddress=return_address+PAGE_SIZE*numOfPages;
}

uint32 best_fit_strategy(int size,int numOfPages)
{
	int bestsizeIndex=p_alloc[0].index;
	int bestSize=p_alloc[0].framesNum;
	for(int k=0;k<=bestIndex;k++)
	{
		if(p_alloc[k].framesNum<bestSize)
		{
			bestSize=p_alloc[k].framesNum;
			bestsizeIndex=p_alloc[k].index;
		}
	}
	alloc[bestsizeIndex].framesNum=numOfPages;
	uint32 return_address=alloc[bestsizeIndex].startAddress;
	sys_allocateMem(return_address,size);
	counterOfAlloc++;
	updateAllocArray(bestsizeIndex,counterOfAlloc,numOfPages,return_address);
	return return_address;
}

void alloceted(int counterOfAlloc,uint32 start_address,int numOfPages,uint32 size ){


	alloc[counterOfAlloc].startAddress=start_address;
	alloc[counterOfAlloc].framesNum=numOfPages;
	sys_allocateMem(start_address,size);
}

void merge_block(int counterOfAlloc ){

	for(int i=0;i<=counterOfAlloc;i++)
	{
		if(alloc[i].framesNum==0)
		{
			int numToShiftLeft=0;
			for(int j=i+1;j<=counterOfAlloc&&alloc[j].framesNum==0;j++)
			{
				numToShiftLeft++;
			}
			for(int k=i+1+numToShiftLeft;k<=counterOfAlloc;k++)
			{
				alloc[k-numToShiftLeft].framesNum=alloc[k].framesNum;
				alloc[k-numToShiftLeft].startAddress=alloc[k].startAddress;
			}
		}
	}
}

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2021 - [2] User Heap] malloc() [User Side]
	bestIndex=-1;
	uint32 return_address;
	size=ROUNDUP(size,PAGE_SIZE);
	uint32 numOfPages=size/PAGE_SIZE;

	if(checkMemValid(numOfPages,counterOfAlloc)==NULL){
		return NULL;
	}
	merge_block(counterOfAlloc);
	if(counterOfAlloc==-1)
	{
		counterOfAlloc++;
		return_address =start_address;
		alloceted( counterOfAlloc, start_address,numOfPages,size );
		start_address=start_address+numOfPages*PAGE_SIZE;
		return (void*)return_address;
	}
	else
	{
		int fEmpty=0;
		for(int i=0;i<=counterOfAlloc;i++)
		{
			if(alloc[i].framesNum==0)
			{
				fEmpty=1;
				break;
			}
		}
		if(fEmpty==1)
		{
			for(int i=0;i<=counterOfAlloc;i++)
			{
				if(alloc[i].framesNum==0)
				{
					int freeSize=(alloc[i+1].startAddress-alloc[i].startAddress)/PAGE_SIZE;
					if(freeSize==numOfPages)
					{
						alloc[i].framesNum=numOfPages;
						return_address=alloc[i].startAddress;
						sys_allocateMem(return_address,size);
						return (void *)return_address;
					}
					if(freeSize>numOfPages)
					{
						bestIndex++;
						p_alloc[bestIndex].framesNum=freeSize;
						p_alloc[bestIndex].index=i;
					}
				}
			}
		}
		if(fEmpty==0||bestIndex==-1)
		{
			counterOfAlloc++;
			return_address =start_address;
			alloceted( counterOfAlloc, start_address,numOfPages,size );
			start_address=start_address+numOfPages*PAGE_SIZE;
			return (void*)return_address;
		}
		return (void*)best_fit_strategy(size,numOfPages);
	}
	return NULL;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	//TODO: [PROJECT 2021 - [2] User Heap] free() [User Side]
	int index = -1;
	for(index = 0; index<counterOfAlloc; index++){
		if(alloc[index].startAddress == (uint32)virtual_address){
			break;
		}
	}
	sys_freeMem((uint32)virtual_address, alloc[index].framesNum*PAGE_SIZE);
	alloc[index].framesNum = 0;

}

//==================================================================================//
//================================ OTHER FUNCTIONS =================================//
//==================================================================================//

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	panic("this function is not required...!!");
	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	panic("this function is not required...!!");
	return 0;
}

void sfree(void* virtual_address)
{
	panic("this function is not required...!!");
}

void *realloc(void *virtual_address, uint32 new_size)
{
	panic("this function is not required...!!");
	return 0;
}

void expand(uint32 newSize)
{
	panic("this function is not required...!!");
}

void shrink(uint32 newSize)
{
	panic("this function is not required...!!");
}

void freeHeap(void* virtual_address)
{
	panic("this function is not required...!!");
}



