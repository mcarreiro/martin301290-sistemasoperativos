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
			cout << "Hola" << *it<<endl;
			bloqueados.erase(it);
			break;
		}
	}
    
}

int SchedRR::tick(const enum Motivo m) {  	
	switch(m){
		case EXIT: return haceExit(m);		
		case BLOCK: return haceBlock(m);		
		case TICK: return haceTick(m);
	}
	
}

int SchedRR::haceTick(const enum Motivo m) {
	// Tenemos que ver si se estaba ejecutando la IDLE o no...
			if (current_pid() == IDLE_TASK)	// Caso que se estaba ejecutando la IDLE
			{
				//Si no tengo procesos devuelvo la idle ...
				if (q.empty())	{
					return IDLE_TASK;
				}
				
				//Si tengo procesos devuelvo alguno que no este bloqueado ...
				int tamCola = q.size();
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
				
					int tamCola = q.size();
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
	
}

int SchedRR::haceExit(const enum Motivo m) {
	// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) {
			return IDLE_TASK;
		} else {		
			contador = 0;			
			int tamCola = q.size();
			for(int i = 0; i < tamCola;i++)
				{
						int sig = q.front(); q.pop();
						if(NoEstaBloqueado(sig))
							{								
								return sig;
						}else{
							q.push(sig);
						}
							
				}
				return IDLE_TASK;
		}
}

int SchedRR::haceBlock(const enum Motivo m) {
	if(NoEstaBloqueado(current_pid())){
				bloqueados.insert(bloqueados.begin(), current_pid());
			}
			contador = 0;
			int tamCola = q.size();
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
	
	
	
