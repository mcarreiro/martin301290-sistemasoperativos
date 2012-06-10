#include "RWLockSemaphoreLock.h"

RWSemaphoreLock :: RWLock(){
	
	//Inicializo mutexReaders en algun valor
	pthread_mutex_init(&mutexReaders, NULL); 
	
	//Me aseguro que lo inicializo en 1
	pthread_mutex_unlock(&mutexReaders);			
					
	//Inicializo el semaforo para saber si hay escritores en 1
	sem_init(&writers, 0, 1);		
	
	//Inicializo el semaforo para ver si el recurso esta libre en 1
	sem_init(&freeResource, 0, 1);
	
	//cantidad de lectores empieza en 0, of course
	readersQuantity = 0;
}
void RWLock :: wunlock(){
	//libero el recurso y pongo en 0 la cantidad de escritores
				
	sem_signal(&freeResource);		
	sem_signal(&writers);
}
void RWLock :: rlock(){
	
	sem_wait(&writers);				/* Si cuando se hace un lock de lectura, hay alguno de escritura esperando, el de lectura se va a quedar esperando en este wait, y va a continuar cuando se libere la escritura */
	sem_signal(&writers);			/* Este signal lo hago porque no quiero que la lectura me bloquee a que lleguen pedidos de escritura */
	
    pthread_mutex_lock(&readersQuantity);		/* Pido el mutex para ver la cantidad de lectores */
    readersQuantity++;						/* Digo que hay un nuevo lector */
    if(readersQuantity == 1)					/* Si hay un solo lector... */
    	sem_wait(&freeResource);		/* ... es porque soy el primer lector en entrar. Entonces digo que el recurso esta ocupado (para lo escritores, los lectores pueden seguir entrando) */
	
	pthread_mutex_unlock(&readersQuantity)	/* Libero el mutex */    
}
void RWSemaphoreLock :: runlock(){
	//pido mutex para la sección crítica
	pthread_mutex_lock(&mutexReaders)
	//decremento la cantidad de lectores		
	readersQuantity--;
	//si no quedan mas lectores libero el recurso
	if(readersQuantity == 0)					
		sem_post(&freeResource);
	
	pthread_mutex_unlock(&mutexReaders);
}
