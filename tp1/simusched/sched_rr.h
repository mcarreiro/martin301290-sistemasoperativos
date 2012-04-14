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
		bool NoEstaBloqueado(int pid);
		int haceTick(const enum Motivo m);
		int haceBlock(const enum Motivo m);
		int haceExit(const enum Motivo m);

		
		int quantum;
		std::queue<int> q;
		int contador;
		std::vector<int> bloqueados;

};

#endif
