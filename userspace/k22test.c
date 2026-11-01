#include <linux/k22info.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*Find parent returns the position on the stack that the parent was found.
 *Due to how the stack functions, when we push a process pid into it ,
 *we have already pushed the pids of it's parent and it's ancestors.
 *As such we are ready to go back as far as needed to figure out the depth
 *where the current process is at
 */
int find_parent(int *stack ,int top,int parent){
	/*While the stack still has entries and we havent reached the parent, pop*/
	while(top >= 0 && stack[top] != parent){
		top--;
	}
	return top;
}


int main(void){
	int value = 4;				//Original buffer size
	struct k22info *buf=malloc(sizeof(*buf)*(value));
	if (!buf) {
		printf("Initial allocation failed");
		return 1;
	}

	int *ne = &value;
	int sys_return=syscall(472,buf,ne);

	printf("- User-space buf. size:  %d\n",value);
	while(sys_return == -1){
		if( errno == EINVAL){					//Found more processes than we had size for
			printf("- syscall return val:   %d (size: %d)\n",sys_return ,value*2 );
			value*=2;
			struct k22info *temp = realloc(buf,value*(sizeof(struct k22info)));
			if(!temp){
				printf("Reallocation failed");
				free(buf);
				return -ENOMEM;				//An allocatio nfailed
			}
			buf=temp;
		} else if(errno == EFAULT) {		//Internal workings of the syscall failed
			printf("Failed to update size or buffer to user space\n");
			free(buf);
			return -EFAULT;
		}else if(errno == ENOMEM){			//An allocation failed inside the syscall
			printf("Reallocation failed");
			free(buf);
			return -ENOMEM;
		}
		printf("- User-space buf. size:  %d\n",value);
		sys_return=syscall(472,buf,ne);					//Try again to call the function
	}
	printf("- syscall return val:   %d (size: %d)\n",sys_return ,value);
	printf("--- OK ---\n\n");
	printf("#comm,pid,ppid,fcldpid,nsblpid,nvcsw,nivcsw,stime,userpc,kernelpc\n");
	struct k22info *p=&buf[0];
	printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",p->comm,p->pid,p->parent_pid,p->first_child_pid,p->next_sibling_pid,p->nvcsw,p->nivcsw,p->start_time/1000000000,(void *)p->userpc,(void *)p->kernelpc);

	int top = -1; //Empty stack;
	int *stack = malloc(value*sizeof(int));
	if(!stack) {
		printf("Stack allocation failed");
		free(buf);
		return ENOMEM;
	}
	stack[0]=buf[0].pid;	//Initialization with the pid of the swapper since we printed it
	top++; //1 element

	for( int i=1; i<*ne;i++){
		p = &buf[i];
		int p_pos = find_parent(stack,top,p->parent_pid);
		for(int j=0; j<p_pos+1; j++){
			printf("-");
		}
		printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",p->comm,p->pid,p->parent_pid,p->first_child_pid,p->next_sibling_pid,p->nvcsw,p->nivcsw,p->start_time/1000000000,(void *)p->userpc,(void *)p->kernelpc);
		top=p_pos+1;
		stack[top]=p->pid;	//We push the current process for the future;
	}
	free(buf);
	free(stack);
	return 0;
}