#include <linux/get_pc.h>
#include <arch/arm64/include/asm/processor.h>



unsigned long get_pc(struct task_struct *process)
{
	return process->thread.cpu_context.pc;


}