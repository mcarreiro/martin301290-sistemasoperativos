#include "RWSemaphoreLock.h"

RWSemaphoreLock :: RWSemaphoreLock(){
	
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
void RWSemaphoreLock :: rlock(){
	//veo que no haya nadie escribiendo
	sem_wait(&writers);
	//inhibo el bloqueo para pedidos de escritura
	sem_post(&writers);
	
    pthread_mutex_lock(&mutexReaders);		
    
    readersQuantity++;//aumento la cantidad de lectores
    if(readersQuantity == 1)					
    	sem_wait(&freeResource);//si soy el primer lector tomo el recurso
	
	pthread_mutex_unlock(&mutexReaders);
}
void RWSemaphoreLock :: wunlock(){
	//libero el recurso y pongo en 0 la cantidad de escritores
				
	sem_post(&freeResource);		
	sem_post(&writers);
}

void RWSemaphoreLock :: wlock(){
	//espero que no haya nadie escribiendo y tomo el recurso
	sem_wait(&writers);
    sem_wait(&freeResource);
}

void RWSemaphoreLock :: runlock(){
	//pido mutex para la sección crítica
	pthread_mutex_lock(&mutexReaders);
	//decremento la cantidad de lectores		
	readersQuantity--;
	//si no quedan mas lectores libero el recurso
	if(readersQuantity == 0)					
		sem_post(&freeResource);
	
	pthread_mutex_unlock(&mutexReaders);
}
