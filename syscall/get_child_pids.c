#include <linux/linkage.h>
#include <linux/sched.h>
#include <linux/rwlock.h>
#include <linux/uaccess.h>

asmlinkage long sys_get_child_pids(pid_t* list, size_t limit, size_t* num_children){
	struct task_struct *task;
	struct list_head* task_list;
	      
	size_t counter=0;
	write_lock_irq(&tasklist_lock);
	int err=0;
	 if (!list && !limit) 
	 	err = EFAULT;


	list_for_each(task_list, &current->children){

	 	  counter=counter+1;
	 	  
	      if(limit>=counter && err==0){
	      	task=list_entry(task_list, strcut task_struct, sibling);
	      	err=put_user(task->pid,list);
	        list++;
	    }    	 
	  
	}
	write_unlock_irq(&tasklist_lock);
	int err2 = put_user(counter,num_children);
	if(err == 0)
		err = err2;
	if(counter>limit){
		return -ENOBUFS;
	}
	return -err;	
}