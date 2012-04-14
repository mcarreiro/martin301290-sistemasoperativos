#include <vector>
#include <queue>
#include "sched_lottery.h"
#include "basesched.h"

using namespace std;

SchedLottery::SchedLottery(vector<int> argn) {
	srand((unsigned int) argn[0]);
	quantum = (unsigned int) argn[1];
	contador = 0;
	ticketsTotal = 0;	
}

void SchedLottery::load(int pid) {
	tarea nuevaTarea(pid,1,NORMAL);
	q.push_back(nuevaTarea);					// Lo agrego a la cola de procesos
	ticketsTotal++;	
}

void SchedLottery::unblock(int pid) {
	int i = buscarProcesoEnCola(pid);
	q[i].estado = SOBRAN_TICKETS;
	ticketsTotal += q[i].tickets;	// Recupero los tickets del proceso bloqueado para que entre en el sorteo

}

int SchedLottery::tick(const enum Motivo m) {
	switch(m)
	{
		case BLOCK: return haceBlock();
		case TICK:  return haceTick();
		case EXIT:  return haceExit();
	}		
}

int SchedLottery::haceBlock(){
	contador ++;	//Esto es porque al menos uso uno del CPU y para darle la compensacion no puede ser cero
	int i = buscarProcesoEnCola(current_pid());
	ticketsTotal -= q[i].tickets;
	q[i].tickets += quantum / contador; // Le asigno la compensacion definida como 1/(contador/quantum) = quantum/contador
	q[i].estado = BLOQUEADO; // esta bloqueado y tiene tickets de mas, este es el estado de bloqueado

	if (ticketsTotal > 0) { //Quedan tareas en la cola de activas
		int ticketGanador = random() % ticketsTotal;			// Consigo el ticket ganador
		contador = 0; 							// Reseteo contador para el proximo
		int indiceGanador = sorteo(ticketGanador);
		sacarTicketsDeMas(indiceGanador);
		return q[indiceGanador].pid;
	}else{
		return IDLE_TASK;	
	}
}

int SchedLottery::haceTick(){
	contador ++;
	if ((quantum == contador || current_pid() == IDLE_TASK) && ticketsTotal > 0) { //Debo cambiar de tarea y hay más para hacerlo
		int ticketGanador = random() % ticketsTotal;			// Consigo el ticket ganador
		contador = 0; 							// Reseteo contador para el proximo
		int indiceGanador = sorteo(ticketGanador);
		sacarTicketsDeMas(indiceGanador);
		return q[indiceGanador].pid;
	}else if (quantum != contador){ 			// Todavia no terminé sigo, en caso de que haya terminado el tiempo pero no la tarea y sea la
		return current_pid();				// última entra en el caso anterior y el sorteo lo gana el que está solo;
	}else{							// No tengo ni tareas 
		return IDLE_TASK;
	}
}

int SchedLottery::haceExit(){
	int i = buscarProcesoEnCola(current_pid());
	ticketsTotal -= q[i].tickets;
	q.erase(q.begin() + i);
	if (ticketsTotal > 0) { //Quedan tareas en la cola de activas
		int ticketGanador = random() % ticketsTotal;			// Consigo el ticket ganador
		contador = 0; 							// Reseteo contador para el proximo
		int indiceGanador = sorteo(ticketGanador);			// Obtengo el proximo que tiene que correr
		sacarTicketsDeMas(indiceGanador);
		return q[indiceGanador].pid;
	}else{
		return IDLE_TASK;	
	}
}

int SchedLottery::buscarProcesoEnCola(int pid) {
	int i = 0;
	while(true)
	{
		if(q[i].pid == pid)
		{
			return i;						// Asumimos que el proceso que busco esta en la cola por precondicion
		}
		i++;
	}
}

int SchedLottery::sorteo(int ticketGanador){
		int sumaTickets = 0;
		int i = 0;
		while(true)
		{
			if(q[i].estado != BLOQUEADO)				
			{
				sumaTickets += q[i].tickets;			// Sumo los tickets hasta llegar al que salió
				if(sumaTickets > ticketGanador) break;		// Es mayor, porque si sale cero se le asigna al primero y si sale n-1 se le asigna al ultimo
			}							// Si fuera mayor o igual, estariamos asignando 2 al primero y cero al ultimo
			i++;
		}
		return i;
}

void SchedLottery::sacarTicketsDeMas(int indiceGanador){
	if (q[indiceGanador].estado == SOBRAN_TICKETS){			// Si fue compensado hay que sacarle los que tiene de más
			ticketsTotal = ticketsTotal - q[indiceGanador].tickets - 1;				
			q[indiceGanador].tickets = 1;
	}
}
