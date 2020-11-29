# Entwurf

## Control Thread
Controls producers and consumers by using a mutex for each thread:

Producer 1: `mtx_p1`
Producer 2:`mtx_p2`
Consumer: `mtx_c`

The mutexes are passed as a parameter to the threads.
Once the control thread locks the corresponding mutex,
the consumer  producer thread will lock and only continue once the mutex is unlocked by the control thread.


## Producer and Consumer Threads
Checks the corresponding mutex `mtx_p1`, `mtx_p2` or `mtx_c` before inserting / deleteing an element.
The thread can call the functions to add or remove an element without checking for other threads,
as the buffer will handle threading itself.


## FIFO
- Implementation of buffer as queue or circular buffer


### Variant 1 Semaphore
The buffer  contains a mutex and two semaphores:
- `mtx` the mutex
- `sem_used` semaphore with val = count of elements in buffer structure
- `sem_free` semaphore with val = remaining capacity in buffer structure

##### Add element
1. create element `el`
1. check `sem_wait` for `sem_free`
    - only allows to continue when there is at least 1 capacity left
    - decreases the count of free spaces when continuing
1. lock `mtx` 
1. add `el` to buffer
1. unlock `mtx` 
1. call `sem_post` for `sem_used`
    - increases the count of elements in buffer
    - wake threads wanting to remove an element from the buffer (waiting on `sem_used`)

##### Remove element
1. check `sem_wait` for `sem_used`
    - only allows to continue when there is at least 1 element in buffer
    - decreases the count of used spaces when continuing
1. lock `mtx` 
1. remove oldest `el` from buffer
1. unlock `mtx` 
1. call `sem_post` for `sem_free`
    - increases the count of free spaces in buffer
    - wakes threads wanting to add an element to the buffer (waiting on `sem_free`)
1. return `el`


### Variant 2 CondVars
The buffer contains two integer values, a mutex and two semaphores:
- `int capacity` the maximum capacity of the buffer
- `int size` the current size of the buffer
- `mtx` the mutex
- `cv_isEmpty` condition variable indicating buffer is empty
- `cv_isFull` condition variable indicating buffer is full

##### Add element
1. create element `el`
1. lock `mtx`
1. check if the buffer is full
    - if yes:`cond_wait` with `cv_full` and `mtx` in loop 
    - if no: continue with execution  
1. increment size
1. call `cond_signal` on `cv_empty`, waking threads wanting to remove an element.
1. add `el` to buffer
1. unlock `mtx`

##### Remove element
1. lock `mtx` 
1. check if the buffer is is empty
    - if yes:`cond_wait` with `cv_empty` and `mtx` in loop 
    - if no: continue with execution  
1. decrement size
1. call `cond_signal` on `cv_full`, waking threads wanting to add an element.
1. remove oldest element `el` from buffer
1. unlock `mtx`
1. return `el` 
