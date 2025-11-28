#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <string>
#include <iostream>
#include "coroutine.h"

using namespace std;

struct args_t
{
	int id;
	std::string message;
};
void* routine_func(void* args)
{
	args_t* arg = (args_t*)args;
	for (int i = 0; i < 5; ++i)
	{
		printf("Routine %d: %s %d\n", arg->id, arg->message.c_str(), i);
		co_yield_ct();
	}
	return NULL;
}
void* routine_func2(void* args)
{
	for (int i = 0; i < 5; ++i)
	{
		printf("---------------------\n");
		co_yield_ct();
	}
    printf("routine_func2 end\n");
	return NULL;
}

void test_shared_stack()
{
	const int routine_count = 5;
	stCoRoutine_t* routines[routine_count];
	args_t args[routine_count];
	stCoRoutineAttr_t attr{};
	stShareStack_t* share_stack = co_alloc_sharestack(1, 1024 * 128);
	attr.share_stack = share_stack;

	for (int i = 0; i < routine_count; ++i)
	{
		args[i].id = i;
		args[i].message = "Hello from shared stack";
		if(i % 2 == 0)
			co_create(&routines[i], &attr, routine_func2, &args[i]);
		else
			co_create(&routines[i], &attr, routine_func, &args[i]);
	}

	for(int j = 0; j <= 5; ++j)
	for (int i = 0; i < routine_count; ++i)
		co_resume(routines[i]);

}

int main()
{
	test_shared_stack();
	return 0;
}