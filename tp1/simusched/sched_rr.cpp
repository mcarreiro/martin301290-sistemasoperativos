#include <vector>
#include <queue>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;


SchedRR::SchedRR(vector<int> argn) {
	quantum = argn[0]; 	//Recibe quantum
	contador = 0;		//Inicializo el contador
}

void SchedRR::load(int pid) {
	q.push(pid); // llegó una tarea nueva
}

void SchedRR::unblock(int pid) {	
	//Para desbloquear recorro el vector de bloqueados con un iterador.
	//Si lo encuentra lo borra, desbloqueandolo.
	vector<int>::iterator it;
	for ( it= bloqueados.begin() ; it < bloqueados.end(); it++ ){
		if(*it == pid){
			bloqueados.erase(it);
			break;
		}
	}
    
}

int SchedRR::tick(const enum Motivo m) {  	
	switch(m){
		case EXIT: return haceExit(m);		//CASO EXIT	
		case BLOCK: return haceBlock(m);	//CASO BLOCK
		default: return haceTick(m); 		//CASO TICK
	}	
}

int SchedRR::haceTick(const enum Motivo m) {
			// Si se esta ejecutando la idle ...
			if (current_pid() == IDLE_TASK)
			{
				//Si no tengo procesos devuelvo la idle ...
				if (q.empty())	{
					return IDLE_TASK;
				}
				
				//Si tengo procesos busco el proximo proceso a ejecutar
				return ProximoProcesoAEjecutar(false);

			}
			else 
			{
				contador ++;				// Aumentamos el quantum consumido del proceso actual
				if(contador == quantum)		// Si alcanzo el quantum maximo...
				{	
					contador = 0;					// Reseteo el quantum				
					return ProximoProcesoAEjecutar(true); 	//Busco el proximo proceso a ejecutar
				}
				return current_pid();				// Si no se alcanzo el quantum se sigue ejecutando el current
			}		
	
}

int SchedRR::haceExit(const enum Motivo m) {
	// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) {					//Si no quedan mas procesos paso al IDLE
			return IDLE_TASK;
		}
								//Si quedan mas procesos...
		contador = 0;					//Reseteo el quantum
		return ProximoProcesoAEjecutar(false); //Busco el proximo proceso a ejecutar
		
}

int SchedRR::haceBlock(const enum Motivo m) {
	if(EstaBloqueado(current_pid()) == false){ 					//Me fijo que no este bloqueada
		bloqueados.insert(bloqueados.begin(), current_pid());	//Bloqueo el proceso actual
	}
	contador = 0;					//Reseteo el quantum
	return ProximoProcesoAEjecutar(true);	//Busco el proximo proceso a ejecutar
} 

bool SchedRR::EstaBloqueado(int pid)
{	//Recorro la lista de bloqueadas para ver si esta este pid
	for(unsigned int i=0;i < bloqueados.size();i++){
			if(bloqueados[i] == pid){ //Si lo encuentro devuelvo true y fin
				return true;
			}
	}
	return false; //Si no esta bloqueado devuelvo false
}
	
int SchedRR::ProximoProcesoAEjecutar(bool pushearActual){
	int tamCola = q.size();
	for(int i = 0; i < tamCola;i++) 		//Recorro la lista de procesos...
	{
			int sig = q.front(); q.pop();	//Recupero el primero de la cola
			if(!EstaBloqueado(sig))			//Si no esta bloquedo...
				{
					if(pushearActual){		//Si tengo q meter el actual
						q.push(current_pid());	//Lo pusheo...
					}
					return sig;				//Lo devuelvo para seguir con este proceso
				}
			else{
				q.push(sig);				//Lo pusheo y sigo...
			}
				
	}
	return IDLE_TASK;						//Si todas estan bloqueadas o no hay procesos devuelvo la IDLE
}
	
