#ifndef __SCHED_LOTTERY__
#define __SCHED_LOTTERY__

#include <vector>
#include <queue>
#include "basesched.h"

class SchedLottery : public SchedBase {
	public:
		SchedLottery(std::vector<int> argn);
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(const enum Motivo m);
	
	private:
		int ticketsTotal;
		int quantum;
		int contador;
		std::vector<int> ticket;
		std::vector<int> bloqueados;
		std::vector<int> tieneTicketsDeMas;
};

#endif
