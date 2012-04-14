#include "tasks.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

void TaskCPU(vector<int> params) { // params: n
	uso_CPU(params[0]); // Uso el CPU n milisegundos.
}

void TaskCon(vector<int> params) { // params: n
	int i = 0;
	int cant = params[0];
	int lowest=params[1], highest=params[2];
	int random_integer = lowest + rand() % (highest +1 - lowest) ;
	while(i< cant){		
		uso_IO(random_integer);
		i++;
	}

}

void TaskBatch (vector<int> params){
	int tot = params[0];
	int blocks = params[1];
	int subCycles = tot / blocks;
	int random;
	int from;
	int to;
	srand ( time(NULL) );
	for(int i=0;i<tot;i++){
			if(i%subCycles==0){
				from = i;
				to   =  i + subCycles-1;
				random = from + rand() % (to +1 - from) ;
				
			}
			if(random == i){				
				uso_IO(1);				
			}else{				
				uso_CPU(1);
			}
	}
}

void TaskIO(vector<int> params) { // params: ms_cpu, ms_io,
	uso_CPU(params[0]); // Uso el CPU ms_cpu milisegundos.
	uso_IO(params[1]); // Uso IO ms_io milisegundos.
}

void TaskAlterno(vector<int> params) { // params: ms_cpu, ms_io, ms_cpu, ...
	for(int i = 0; i < (int)params.size(); i++) {
		if (i % 2 == 0) uso_CPU(params[i]);
		else uso_IO(params[i]);
	}
}



void tasks_init(void) {
	/* Todos los tipos de tareas se deben registrar acá para poder ser usadas.
	 * El segundo parámetro indica la cantidad de parámetros que recibe la tarea
	 * como un vector de enteros, o -1 para una cantidad de parámetros variable. */
	register_task(TaskCPU, 1);
	register_task(TaskIO, 2);
	register_task(TaskAlterno, -1);
	register_task(TaskCon, 3);
	register_task(TaskBatch, 2);
}
