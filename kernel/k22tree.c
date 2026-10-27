#include <linux/k22info.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/processor.h>
#include <linux/ptrace.h>
#include <errno-base.h>
/*
 *Get_info: takes a the current proccess, the buffer
 *and the entry number in order to fill the buffer.
 *
 */



void get_info(struct task_struct *c_process, struct k22info *buffer, int entry)
{
	int i = 0;

	while (c_process->comm[i] != '\0') {
		buffer[entry].comm[i] = c_process->comm[i];
		i++;
	}
	buffer->comm[i] = '\0';

	buffer[entry].pid = c_process->pid;
	buffer[entry].parent_pid = c_process->parent->pid;
	struct task_struct *child;

	list_for_each_entry(child, &c_process->children, children) {
		if(child==NULL){

		}
		if (child->exit_state != EXIT_ZOMBIE) {
			buffer[entry].first_child_pid = child->pid;
			break;
		}
	}
	struct task_struct *nsibling;
	int flag = 0;

	list_for_each_entry(nsibling, &nsibling->sibling, sibling) {
		if (nsibling == c_process)
			flag = 1;
		if ((nsibling->exit_state != EXIT_ZOMBIE) && flag) {
			buffer[entry].next_sibling_pid = nsibling->pid;
			break;
		}
	}
	buffer[entry].nvcsw = c_process->nvcsw;
	buffer[entry].nivcsw = c_process->nivcsw;
	buffer[entry].start_time = c_process->start_time;
	struct pt_regs *regs = task_pt_regs(c_process);

	if (regs)
		buffer[entry].userpc = regs->ip;
	else
		return -1;


}



int k22tree(struct k22info *buf, int *ne)
{
	rcu_read_lock();            //Lock in order to read the process
	struct task_struct *psudo_init = find_task_by_vpid(0);

	if (psudo_init == NULL)
		return -1;
	rcu_read_unlock();          //Unlock after reading the lock
	return 0;
}