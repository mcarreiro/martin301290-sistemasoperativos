#include <vector>
#include <queue>
#include "sched_lottery.h"
#include "basesched.h"

using namespace std;

SchedLottery::SchedLottery(vector<int> argn) {
	quantum((unsigned int)argn[1]);
	
}

void SchedLottery::load(int pid) {
}

void SchedLottery::unblock(int pid) {
}

int SchedLottery::tick(const enum Motivo m) {
}
