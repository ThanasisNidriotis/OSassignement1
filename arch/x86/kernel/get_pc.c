// SPDX-License-Identifier: GPL-2.0
#include <linux/get_pc.h>
#include <asm/switch_to.h>

unsigned long get_pc(struct task_struct *process)
{
	if (task_curr(process) || process->pid == 0)		//If currently active or is the swapper
		return 0;
	/*It's currently inactive so we can use the struct inactive_task_frame*/
	struct inactive_task_frame *frame = (struct inactive_task_frame *)process->thread.sp;

	return frame->ret_addr;
}
