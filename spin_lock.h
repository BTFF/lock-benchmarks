#ifndef __spin_lock_h__
#define __spin_lock_h__ __spin_lock_h__

#define spin_lock(p) while(!__sync_bool_compare_and_swap((p), 0, 1))
#define spin_unlock(p) do {  asm volatile (""); (*(volatile int*)(p)) = 0; } while(0)

#endif
