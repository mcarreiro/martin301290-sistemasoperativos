#ifndef __SCHED_RR__
#define __SCHED_RR__

#include <vector>
#include <queue>
#include "basesched.h"

class SchedRR : public SchedBase {
	public:
		SchedRR(std::vector<int> argn);
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(const enum Motivo m);
	
	private:
		int next(); 
		
		int quantum;
		std::queue<int> q;
		int contador;
		int bloqueada;

};

#endif
