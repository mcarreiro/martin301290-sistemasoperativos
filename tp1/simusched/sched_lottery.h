#ifndef __SCHED_LOTTERY__
#define __SCHED_LOTTERY__

#include <vector>
#include <algorithm>
#include <queue>
#include "basesched.h"

#define NORMAL -1
#define BLOQUEADO 0
#define SOBRAN_TICKETS 1

//NORMAL = no esta bloqueado y no debe nada, esto es el estado despues de que salio sorteado o que nunca se bloqueó o quue ya devolvió
//BLOQUEADO = esta bloqueado y tiene tickets de mas, este es el estado de bloqueado
//TICKETS_SOBRAN = no esta bloqueado pero tiene tickets de mas que solo sirven para los n rounds hasta que sale sorteado, este es el estado cunado desbloquea

class SchedLottery : public SchedBase {
	public:
		SchedLottery(std::vector<int> argn);
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(const enum Motivo m);
		int haceTick();
		int haceBlock();
		int haceExit();
		int buscarProcesoEnCola(int pid);
		int sorteo(int ticketGanador);
		void sacarTicketsDeMas(int pidGanador);
	
	private:
		
		struct tarea
		{
			tarea() {};
			tarea(int pid, int tickets, int estado) : pid(pid), tickets(tickets), estado(estado) {};
	
			int pid;
			int tickets;
			int estado;
		};


		std::vector<tarea> q;
		int ticketsTotal;
		int quantum;
		int contador;
};

#endif
