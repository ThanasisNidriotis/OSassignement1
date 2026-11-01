#include <linux/get_pc.h>
#include <asm/switch_to.h>

unsigned long get_pc(struct task_struct *process)
{
	struct inactive_task_frame *frame = (struct inactive_task_frame *)process->thread.sp;
	return frame->ret_addr;


}