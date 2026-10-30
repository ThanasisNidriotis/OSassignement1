#include <linux/k22info.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/list.h>
#include <linux/rwlock.h>
#include <linux/processor.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/sched/task_stack.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/timekeeping.h>
#include <asm/thread_info.h>


/*
 *Get_info: takes a the current proccess, the buffer
 *and the entry number in order to fill the buffer.
 *
 */



int get_info(struct task_struct *c_process, struct k22info *buffer, int entry)
{
	int i = 0;
	while (c_process->comm[i] != '\0') {
		buffer[entry].comm[i] = c_process->comm[i];
		i++;
	}
	buffer[entry].comm[i] = '\0';
	buffer[entry].pid = c_process->pid;
	if(c_process->parent){
		buffer[entry].parent_pid = c_process->parent->pid;
	}else{
		buffer[entry].parent_pid=0;
	}
	struct task_struct *child;
	struct list_head *head1 = &c_process->children;
	buffer[entry].first_child_pid=0;
	if (list_empty(head1)){
		buffer[entry].first_child_pid = 0;
	}else{
		list_for_each_entry(child, &c_process->children, sibling) {
			if (child->exit_state==0) {
				buffer[entry].first_child_pid = child->pid;
				break;
			}
		}
	}
	struct task_struct *nsibling=NULL;
	int flag=0;
	buffer[entry].next_sibling_pid=0;
	list_for_each_entry(nsibling, &c_process->parent->children, sibling) {
		if(nsibling == c_process ){
			flag=1;
			continue;
		}
		if ((nsibling->exit_state == 0 )&& flag) {
			buffer[entry].next_sibling_pid = nsibling->pid;
			break;
		}
	}
	buffer[entry].nvcsw = c_process->nvcsw;
	buffer[entry].nivcsw = c_process->nivcsw;
	buffer[entry].start_time = ktime_get_ns()-c_process->start_time;
	struct pt_regs *regs = task_pt_regs(c_process);
	if (regs)
		buffer[entry].userpc = regs->ip;
	else
		buffer[entry].userpc=0;

	buffer[entry].kernelpc=0;
	return 0;
}


int do_k22tree(struct k22info *buf, int *ne)
{
	int x2=3;

	//if(get_user(x2,(int __user *)ne)){
	//	return -EFAULT;
	//}

	struct k22info *temp=kmalloc(sizeof(struct k22info )*(x2),GFP_KERNEL);
	if (temp==NULL){
		return -EFAULT;
	}
	read_lock(&tasklist_lock);           //Lock in order to read the process
	struct task_struct *psudo_init = &init_task;	//Testing with init
	if (psudo_init == NULL) {
		return -2;
		rcu_read_unlock();
	}

	int x=get_info(psudo_init->parent,temp,0);
	 x=get_info(psudo_init,temp,1);
	struct task_struct *child;
	struct list_head *head1 = &psudo_init->children;
	if (list_empty(head1)){
		return -4;
	}else{
		list_for_each_entry(child, &psudo_init->children, sibling) {
			if (child->exit_state==0) {
				break;
			}
		}
	}
	x=get_info(child,temp,2);

	read_unlock(&tasklist_lock);          //Unlock after reading the lock
	copy_to_user(buf,temp,sizeof(struct k22info )*(x2));
	kfree(temp);
	return x;
}

SYSCALL_DEFINE2(k22tree,struct k22info __user*,buf, int *,ne ){
	return do_k22tree(buf,ne);
}