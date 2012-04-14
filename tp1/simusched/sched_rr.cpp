#include <vector>
#include <queue>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;


SchedRR::SchedRR(vector<int> argn) {
	quantum = argn[0]; //Recibe quantum
	contador = 0;
}

void SchedRR::load(int pid) {
	q.push(pid); // llegó una tarea nueva
}

void SchedRR::unblock(int pid) {	
	vector<int>::iterator it;
  
	for ( it= bloqueados.begin() ; it < bloqueados.end(); it++ ){
		if(*it == pid){
			bloqueados.erase(it);
			break;
		}
	}
    
}

int SchedRR::tick(const enum Motivo m) {  	
	int tamCola;
	switch(m){
	case EXIT:
		// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) {return IDLE_TASK;}
		else {		
			contador = 0;			
			tamCola = q.size();
			for(int i = 0; i < tamCola;i++)
				{
						int sig = q.front(); q.pop();
						if(NoEstaBloqueado(sig))
							{								
								return sig;
							}
						else{
							q.push(sig);
						}
							
				}
				return IDLE_TASK;
		}
	break;
	case BLOCK:		
			bloqueados.insert(bloqueados.begin(), current_pid());
			contador = 0;
			tamCola = q.size();
				for(int i = 0; i < tamCola;i++)
				{
						int sig = q.front(); q.pop();
						if(NoEstaBloqueado(sig))
							{
								q.push(current_pid());
								return sig;
							}
						else{
							q.push(sig);
						}
							
				}
				return IDLE_TASK;
			
	break;
	case TICK:
			// Tenemos que ver si se estaba ejecutando la IDLE o no...
			if (current_pid() == IDLE_TASK)	// Caso que se estaba ejecutando la IDLE
			{
				//Si no tengo procesos devuelvo la idle ...
				if (q.empty())	{return IDLE_TASK;}
				
				//Si tengo procesos devuelvo alguno que no este bloqueado ...
				tamCola = q.size();
				for(int i = 0; i < tamCola;i++)
				{
						int sig = q.front(); q.pop();
						if(NoEstaBloqueado(sig)){
							return sig;
						}
						else{
							q.push(sig);
						}
							
				}
				
			}
			else 
			{
				contador ++;				// Aumentamos el quantum consumido del proceso actual
				if(contador == quantum)		// Si alcanzo el quantum maximo...
				{	
					contador = 0;			// Reseteo el quantum				
				
					tamCola = q.size();
					for(int i = 0; i < tamCola;i++)
					{
							int sig = q.front(); q.pop();
							if(NoEstaBloqueado(sig))
							{
								q.push(current_pid());
								return sig;
							}
							else{
								q.push(sig);	
							}						
					}

				}
				return current_pid();	
			}		// Si no se alcanzo el quantum se sigue ejecutando el current
	
	break;
	}
}


bool SchedRR::NoEstaBloqueado(int pid)
{
	for(int i=0;i < bloqueados.size();i++){
			if(bloqueados[i] == pid){
				return true;
			}
	}
	return false;
}
	
	
	
