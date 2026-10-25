#include <linux/k22info.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/rcupdate.h>


int k22tree(struct k22info *buf, int *ne){
    
    rcu_read_lock();
   
    struct task_struct * psudo_init = find_task_by_vpid(1);   //init is the first process, and it's PID is 1 
   
   if(psudo_init==NULL){return -1;}// return -1 == pointer is NULL
   
   struct list_head init_children = psudo_init->children;


    rcu_read_unlock();

    return 0;       //Sucess!
}