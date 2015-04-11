# include <stdio.h>
# include "syscalls.h"
# include <pthread.h>

void *threadFunc()
{
    int uuid = 0;
    long res = get_unique_id(&uuid);
    
    printf ( "Syscall returned %d, uuid is %d\n", res, uuid);
    
}


int main () {
    int i;
    pthread_t pth;
	
    for (i=0; i<10; i++)
    {
        printf("Call n°%d\n", i); fflush(stdout);
        pthread_create(&pth, NULL, threadFunc,NULL);
    }
	
    return 0;
}
