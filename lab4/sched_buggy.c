#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>
#include <kern/pmap.h>
#include <kern/spinlock.h>

void sched_halt(void);
// 剩余的时间片 每次调度RemainingTimeSlice - 1
int RemainingTimeSlice;

// Choose a user environment to run and run it.
void sched_yield(void)
{
	struct Env *idle;

	if (RemainingTimeSlice)
	{
		RemainingTimeSlice -= 1;
	}
	else
	{
		for (int i = 0; i < NENV; i++)
		{
			// 全部设置为最高级
			envs[i].priority = 3;
			envs[i].time_slice = 0;
		}
		// 总的时间片设置为20
		RemainingTimeSlice = 20;
	}

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 4: Your code here.
	// 循环开始的位置
	int start = 0;
	int j;
	// 下一个要运行的env 初始化为一个不可能的值
	int nextENV = NENV;

	if (curenv)
	{
		// 尝试输出信息
		// cprintf("sched:  \tenv_id：%d \tpriority：%d \ttime_slice: %d\n", ENVX(curenv->env_id), curenv->priority, curenv->time_slice);
		// 如果是从一个运行的进程中换下来的，将该进程的时间片-1
		if (curenv->time_slice > 0)
		{
			// 如果还有时间片 时间片-1
			curenv->time_slice -= 1;
		}
		else if (curenv->priority > 0)
		{
			// 没有时间片 降低优先级 然后从新设置时间片
			curenv->priority -= 1;
			switch (curenv->priority)
			{
			case 3:
				curenv->time_slice = 0;
				break;
			case 2:
				curenv->time_slice = 1;
				break;
			case 1:
				curenv->time_slice = 3;
				break;
			} //所有priority == 0 的时间片都设置为0
		}
		start = ENVX(curenv->env_id) + 1;
	}
	for (int i = 0; i < NENV; i++)
	{
		j = (start + i) % NENV;
		if (envs[j].env_status == ENV_RUNNABLE)
		{ //越界bug
			if (nextENV == NENV || (envs[j].priority > envs[nextENV].priority))
			{
				nextENV = j;
			}
		}
	}
	if (nextENV != NENV)
	{
		env_run(&envs[nextENV]);
	}
	if (curenv && curenv->env_status == ENV_RUNNING)
	{
		// 没有新的可运行的环境 接着运行现在的
		env_run(curenv);
	}

	// 无事请可做
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++)
	{
		if ((envs[i].env_status == ENV_RUNNABLE ||
			 envs[i].env_status == ENV_RUNNING ||
			 envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV)
	{
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	// 设置为 halted状态
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile(
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
		// Uncomment the following line after completing exercise 13
		"sti\n"
		"1:\n"
		"hlt\n"
		"jmp 1b\n"
		:
		: "a"(thiscpu->cpu_ts.ts_esp0));
}
