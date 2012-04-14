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
	srand ( time(NULL) );
	int random_integer = lowest + rand() % (highest +1 - lowest) ;
	while(i< cant){		
		uso_IO(random_integer);
		i++;
	}

}

/**
 * Esta tarea subdivide la cantidad total de ciclos en subconjuntos de ciclos
 * la cantidad de subconjuntos es igual o igual+1 a la cantidad de bloqueos que debe realizar
 * los subconjuntos son dijuntos y estan formados por numeros conitguos donde cada numero representa un ciclo
 * Dentro de cada subconjunto elige un ciclo aleatoriamente y en ese ciclo bloquea.
 * Ejemplo:TaskBatch 8 3 
 * sub conjuntos       sub1 sub2  sub3  sub4
 * Ciclos              0 1 | 2 3 | 4 5 | 6 7
 * Ciclo en que bloqua 1   | 2   | 4   |7 (este en realidad no importa porq tiene un contador de bloqueos y este no llegaria a ejecutarlo)
 * */
void TaskBatch (vector<int> params){
	int tot = params[0] ;
	int blocks = params[1];
	int subCycles = tot / blocks;
	int random;
	int from;
	int to;
	int contBlocks = 0;
	srand ( time(NULL) );
	for(int i=0;i<tot;i++){
			//genero ciclo en que voy a bloquear
			if(i%subCycles==0 && contBlocks < blocks){
				from = i;
				to   =  i + subCycles-1;
				random = from + rand() % (to - from) ;
				contBlocks++;
			}
			if(random == i){
				//si llegue al ciclo de bloqueo bloqueo y sumo 1 (ya que el block demora 1 mas la 1 de la simulacion de la llamada)
				uso_IO(1);
				i++;				
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
