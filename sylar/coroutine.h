#pragma once

#include <cstdint>
#include <memory.h>
#include <stdlib.h>

typedef void *(*pfn_co_routine_t)( void * );
typedef void* (*coctx_pfn_t)( void* s, void* s2 );

struct stShareStack_t;
struct stStackMem_t;
struct stCoRoutine_t;
struct stCoRoutineEnv_t;


struct stCoRoutineAttr_t
{
	int stack_size;					//栈大小
	stShareStack_t*  share_stack;	//共享栈，为NULL表示不使用共享栈
	stCoRoutineAttr_t()
	{
		stack_size = 128 * 1024;
		share_stack = NULL;
	}
}__attribute__ ((packed));

//用于维护协程调用栈
struct stCoRoutineEnv_t
{
	stCoRoutine_t *pCallStack[ 128 ];	//协程调用栈
	int iCallStackSize;					//当前的调用深度

	//for copy stack log lastco and nextco
	stCoRoutine_t* pending_co;			//将要切换到的协程
	stCoRoutine_t* occupy_co;			//将要挂起的协程
};

//协程上下文
struct coctx_t
{
	void *regs[ 14 ];	//rip, rsp, rbx, rbp, r12, r13, r14, r15, rdi, rsi, rdx, rcx, r8, r9
	size_t ss_size;		//stack size
	char *ss_sp;		//base addr of stack, maybe not equal to rsp
};

//协程体
struct stCoRoutine_t
{
	stCoRoutineEnv_t *env;		//协程所属的环境
	pfn_co_routine_t pfn;		//协程执行函数
	void *arg;					//协程执行函数参数
	coctx_t ctx;				//用户态保存协程上下文

	char cStart;		//协程是否开始执行
	char cEnd;			//协程是否执行结束
	char cIsMain;		//用于标识主协程
	char cIsShareStack;			//是否使用共享栈

	//char sRunStack[ 1024 * 128 ];
	stStackMem_t* stack_mem;	//运行时栈空间，可能为共享栈、独立栈


	//save satck buffer while confilct on same stack_buffer;
	char* stack_sp; 			//协程运行时栈指针位置
	unsigned int save_size;		//挂起时栈大小
	char* save_buffer;			//挂起时保存栈数据的buffer

};

//协程栈内存块
struct stStackMem_t
{
	stCoRoutine_t* occupy_co;			//占用该栈的协程
	int stack_size;						//栈大小
	char* stack_bp; //stack_buffer + stack_size
	char* stack_buffer;					//栈内存块

};

//池化的共享栈
struct stShareStack_t
{
	unsigned int alloc_idx;			//已分配的栈内存块索引
	int stack_size;					//每个栈内存块大小
	int count;						//栈内存块数量
	stStackMem_t** stack_array;		//全部栈内存块，用于分配
};
extern "C"
{
	extern void swap_context( coctx_t *,coctx_t* ) asm("coctx_swap");
};

stStackMem_t* co_alloc_stackmem(unsigned int stack_size);
stStackMem_t* co_get_stackmem(stShareStack_t* share_stack);
stShareStack_t* co_alloc_sharestack(int count, int stack_size);
void co_init_curr_thread_env();
stCoRoutineEnv_t *co_get_curr_thread_env();
stCoRoutine_t *co_create_env( stCoRoutineEnv_t * env, const stCoRoutineAttr_t* attr, pfn_co_routine_t pfn,void *arg );
int co_create( stCoRoutine_t **ppco,const stCoRoutineAttr_t *attr,pfn_co_routine_t pfn,void *arg );
void co_free( stCoRoutine_t *co );
void co_resume( stCoRoutine_t *co );
void co_yield_env( stCoRoutineEnv_t *env );
void co_yield_ct();
void co_swap(stCoRoutine_t* curr, stCoRoutine_t* pending_co);
int CoRoutineFunc( stCoRoutine_t *co,void * );
int coctx_make(coctx_t* ctx, coctx_pfn_t pfn, const void* s, const void* s1);
int coctx_init(coctx_t* ctx);
void save_stack_buffer(stCoRoutine_t* occupy_co);

enum {
  kEIP = 0,
  kEBP = 6,
  kESP = 7,
};
enum {
  kRDI = 7,
  kRSI = 8,
  kRETAddr = 9,
  kRSP = 13,
};
