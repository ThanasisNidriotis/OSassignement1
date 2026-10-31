#include <linux/get_pc.h>


unsigned long get_pc(struct task_struct *process)
{
	return process->thread.sp + 48 ;

}