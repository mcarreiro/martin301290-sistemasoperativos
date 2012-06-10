#include "RWLockSemaphoreLock.h"

RWSemaphoreLock :: RWLock()
{
	readersQuantity = 0;
	//Inicializo cant_lectores_mutex en algun valor
	pthread_mutex_init(&mutexReaders, NULL); 
	//Me aseguro que lo inicializo en 1
	pthread_mutex_unlock(&mutexReaders);			
	//Inicializo el semaforo para ver si el recurso esta libre en 1
	sem_init(&freeResource, 0, 1);				
	//Inicializo el semaforo para saber si hay escritores en 1
	sem_init(&writers, 0, 1);		
}
