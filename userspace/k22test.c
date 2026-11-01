#include <linux/k22info.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

//printf("- User-space buf. size : %d\n",value);


int main(void){
	int value = 4;
	struct k22info *buf=malloc(sizeof(*buf)*(value));
	if (!buf) {
		printf("Initial allocation failed");
		return 1;
	}
	int *ne = &value;
	int sys_return=syscall(472,buf,ne);
	printf("- User-space buf. size:  %d\n",value);
	while(sys_return == -1){

		if( errno == EINVAL){
			printf("- syscall return val:   %d (size: %d)\n",sys_return ,value*2 );
			value*=2;
			struct k22info *temp = realloc(buf,value*(sizeof(struct k22info)));
			if(!temp){
				printf("Reallocation failed");
				free(buf);
				return -ENOMEM;
			}
			buf=temp;
		} else if(errno == EFAULT) {
			printf("Failed to update size or buffer to user space\n");
			free(buf);
			return -EFAULT;
		}else if(errno == ENOMEM){
			printf("Reallocation failed");
			free(buf);
			return -ENOMEM;
		}
		printf("- User-space buf. size:  %d\n",value);
		sys_return=syscall(472,buf,ne);
	}
	printf("- syscall return val:   %d (size: %d)\n",sys_return ,value );
	printf("--- OK ---\n\n");

	printf("#comm,pid,ppid,fcldpid,nsblpid,nvcsw,nivcsw,stime,userpc,kernelpc\n");
	struct k22info *p=&buf[0];
	printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",p->comm,p->pid,p->parent_pid,p->first_child_pid,p->next_sibling_pid,p->nvcsw,p->nivcsw,p->start_time,p->userpc,p->kernelpc);
	for( int i=1; i<*ne;i++){
		p = &buf[i];
		printf("%s,%d,%d,%d,%d,%lu,%lu,%lu,%p,%p\n",p->comm,p->pid,p->parent_pid,p->first_child_pid,p->next_sibling_pid,p->nvcsw,p->nivcsw,p->start_time,p->userpc,p->kernelpc);

	}
	free(buf);
	return 0;
}