#include <vector>
#include <queue>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;


SchedRR::SchedRR(vector<int> argn) {
	quantum = argn[0]; //Recibe quantum
	contador = 0;
	bloqueada = -2;
}

void SchedRR::load(int pid) {
	q.push(pid); // llegó una tarea nueva
}

void SchedRR::unblock(int pid) {	
	bloqueada = -2; //No hay bloqueada
}

int SchedRR::tick(const enum Motivo m) {  	
	if(bloqueada != -2){			
			unblock(bloqueada);		
			
	}	
	switch(m){
	case EXIT:
		// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) return IDLE_TASK;
		else {
			int sig = q.front(); q.pop();			
			contador = 0;			
			return sig;
		}
	break;
	case BLOCK:		
			bloqueada = current_pid();
			return next();
			
	break;
	case TICK:
			contador += 1;
			if(contador == quantum)
			{	//Se me acabo el quantum, tengo que dejar al siguiente
				return next();
				
			}else{
				return current_pid(); //Sigo YO
			}	
	
	break;
	}
}

int SchedRR::next(){
		if (q.empty()) {			
			return current_pid();
		}
		else {
			int sig = q.front(); q.pop();
			q.push(current_pid());
			contador = 0;			
			return sig;
		}	
		
}		
	
	
	
