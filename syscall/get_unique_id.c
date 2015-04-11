
#include <linux/linkage.h>
#include<asm/atomic.h>
#include<linux/uaccess.h>
#include<linux/init.h>
#include<linux/mutex.h>

static atomic_t v __initdata = ATOMIC_INIT(0); //intialization macro
static DEFINE_MUTEX(cache_lock);

asmlinkage long sys_get_unique_id (int* uuid)
{  
	 
	 mutex_lock(&cache_lock);

     int x = atomic_read(&v);
	 atomic_inc(&v);

	 mutex_unlock(&cache_lock);

    return -put_user(x, uuid);
}
