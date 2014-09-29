CFLAGS += -g -Wall
LFLAGS +=
OBJS = test.o main.o

all: process_none process_spin process_spin_pipe process_fifo process_flock process_mutex process_sem thread_flock thread_spin thread_spin_pipe thread_fifo_pipe thread_mutex thread_sem process_lockf process_peterson process_v_sem thread_lockf thread_peterson thread_v_sem thread_fcntl

process_none: process_none.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_spin: process_spin.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_spin_pipe: process_spin_pipe.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_spin_signal: process_spin_signal.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo: process_fifo.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo_nanosleep: process_fifo_nanosleep.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo_null: process_fifo_null.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo_sync: process_fifo_sync.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo_swap: process_fifo_swap.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_fifo_v_sem: process_fifo_v_sem.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

#process_spin_signal: process_spin_signal.o $(OBJS)
#	$(CC) $(LFLAGS) -o $@ $^

process_flock: process_flock.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

#process_mutex: process_mutex.o $(OBJS)
	#$(CC) $(LFLAGS) -o $@ $^

process_sem: process_sem.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_lockf: process_lockf.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

process_peterson: process_peterson.o $(OBJS) ../peterson/peterson.o
	$(CC) $(LFLAGS) -o $@ $^

process_v_sem: process_v_sem.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

thread_spin: thread_spin.o $(OBJS) 
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_spin_pipe: thread_spin_pipe.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_fifo: thread_fifo.o $(OBJS) 
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_fifo_pipe: thread_fifo_pipe.o $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^

thread_flock: thread_flock.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_mutex: thread_mutex.o $(OBJS) 
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_sem: thread_sem.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_lockf: thread_lockf.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_peterson: thread_peterson.o $(OBJS) ../peterson/peterson.o
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_v_sem: thread_v_sem.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

thread_fcntl: thread_fcntl.o $(OBJS)
	$(CC) $(LFLAGS) -pthread -o $@ $^

clean:
	rm -rf peterson *.o

dep:
	$(CC) $(CFLAGS) -M *.c *.h > .depend

.c.o:
	$(CC) $(CFLAGS) -c $< 

include .depend
