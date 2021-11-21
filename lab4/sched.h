/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

// This function does not return.
void sched_yield(void) __attribute__((noreturn));

#endif	// !JOS_KERN_SCHED_H


/*
exercise 6 笔记：
1. 在inc/env.h 中修改Env数据结构，添加优先级定义 和剩余时间片定义
    如下：
    struct Env {
        ..... 
        int priority;  			// MLFQ 定义的优先级 从0 - 3 共3个优先级
        int time_slice;			// MLFQ 定义的时间片
    }; 

*/
