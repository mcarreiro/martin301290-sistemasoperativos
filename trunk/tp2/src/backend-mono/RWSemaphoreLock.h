#ifndef RWLock_h
#define RWLock_h
#include <iostream>
#include <semaphore.h>

class RWSemaphoreLock {
    public:
        RWLock();
        void rlock();
        void wlock();
        void runlock();
        void wunlock();

    private:
        
        int readersQuantity;
        pthread_mutex_t mutexReaders;
        sem_t freeResource;
        sem_t writers; 
};

#endif
