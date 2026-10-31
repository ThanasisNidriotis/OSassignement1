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
#include <linux/get_pc.h>

int get_info(struct task_struct *c_process, struct k22info *buffer, int* entry,int ne);
struct task_struct *read_child(struct task_struct *curr_p);
struct task_struct *read_parent_sibling(struct task_struct *curr_p);
int do_k22tree(struct k22info *buf, int *ne);

/*
 *Get_info: takes a the current proccess, the buffer
 *and the entry number in order to fill the buffer.
 *
 */


int get_info(struct task_struct *c_process, struct k22info *buffer, int* entry,int ne)
{
	/*If the buffer is full of entries*/
	if((*entry) == (ne))
		return 1;

	int i = 0;
	while (c_process->comm[i] != '\0') {
		buffer[*entry].comm[i] = c_process->comm[i];
		i++;
	}
	buffer[*entry].comm[i] = '\0';
	buffer[*entry].pid = c_process->pid;
	if(c_process->parent){
		buffer[*entry].parent_pid = c_process->parent->pid;
	}else{
		buffer[*entry].parent_pid=0;
	}
	struct task_struct * child = NULL;
	struct list_head *head1 = &c_process->children;
	buffer[*entry].first_child_pid=0;
	if (list_empty(head1)){				//No children
		buffer[*entry].first_child_pid = 0;
	}else{
		list_for_each_entry(child, &c_process->children, sibling) {
			if (child->exit_state==0) {			//First alive child
				buffer[*entry].first_child_pid = child->pid;
				break;
			}
		}
	}
	struct task_struct *nsibling = NULL;
	int flag = 0;

	buffer[*entry].next_sibling_pid = 0;
	list_for_each_entry(nsibling, &c_process->parent->children, sibling) {
		if (nsibling == c_process) {
			flag=1;
			continue;
		}
		if ((nsibling->exit_state == 0) && flag) {
			buffer[*entry].next_sibling_pid = nsibling->pid;
			break;
		}
	}
	buffer[*entry].nvcsw = c_process->nvcsw;
	buffer[*entry].nivcsw = c_process->nivcsw;
	buffer[*entry].start_time = c_process->start_time;
	struct pt_regs *regs = task_pt_regs(c_process);
	if (regs && c_process->mm)			//Has user space memory map AND
		buffer[*entry].userpc = regs->ip;	//we got a pointer to it's stack
	else						//saved registers
		buffer[*entry].userpc = 0;
	buffer[*entry].kernelpc = get_pc(c_process);
	(*entry)++;
	return 0;
}


/*
 *Read_child gets the first alive child of the current_process.
 * Return NULL if no children exist, or a pointer to the next child
 */

struct task_struct *read_child(struct task_struct *curr_p)
{
	struct task_struct *child;

	if (list_empty(&curr_p->children))
		return NULL;

	list_for_each_entry(child, &curr_p->children, sibling) {
		if (child->exit_state == 0)
		return child;
	}
	return NULL;	//all kids are zombies
}

/*
 * read_parent_sibling reads the current process
 * and reads either the next sibling(If any alive exist)
 * and returns it, or returns the parent(And as such goes up the
 * the task process).
 */

struct task_struct *read_parent_sibling(struct task_struct *curr_p)
{
	struct task_struct *sib = NULL;
	int flag = 0;

	list_for_each_entry(sib, &curr_p->parent->children,sibling) {
		if (sib == curr_p) {			//We dont care about older siblings
			flag = 1;
			continue;
		}
		if ((sib->exit_state == 0) && flag)	//Next alive sibling
			return sib;
	}
	return curr_p->parent;			//No other alive siblings so we take the parent



}





int do_k22tree(struct k22info *buf, int *ne)
{
	if ( !buf || !ne || (*ne) <= 0)
		return -EINVAL;

	int num;

	if (get_user(num, (int __user *)ne))
		return -EFAULT;
	/*Temporary buffer*/
	struct k22info *tb = kmalloc(sizeof(struct k22info)*(num), GFP_KERNEL);

	if (!tb)
		return -ENOMEM;
	 /*Lock in order to read the process */
	read_lock(&tasklist_lock);
	struct task_struct *curr = &init_task;
	struct task_struct *next = NULL;
	int x=0;
	int *entry = &x;

	do {
		do {
			do {
				if (next)
					curr = next;
				/*If it returns 1,exceeded entries*/
				if (get_info(curr,tb,entry,num)) {
					read_unlock(&tasklist_lock);
					return -2;
				}
				next = read_child(curr);
			} while(next);
			next = read_parent_sibling(curr);
		} while(next != curr->parent);
		do {
			curr = next;
			next = read_parent_sibling(curr);
		} while(next == curr->parent);
		curr = next;
		next = NULL;
	} while (curr != &init_task);	//While we arent at init loop

	read_unlock(&tasklist_lock);          //Unlock after reading the lock

	if (copy_to_user(buf,tb,sizeof(struct k22info )*(num))) {
		kfree(tb);
		return -EFAULT;
	}

	kfree(tb);
	if (copy_to_user(ne,entry,sizeof(int))) {
		kfree(tb);
		return -EFAULT;
	}
	return 0;
}

SYSCALL_DEFINE2(k22tree,struct k22info __user*,buf, int *,ne ){
	return do_k22tree(buf,ne);
}




