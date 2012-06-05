#!/bin/bash
./simusched lotes/loteEj7.tsk 1 SchedRR 2 >> outputs/ej7/rr-ej7-1-2
./simusched lotes/loteEj7.tsk 1 SchedRR 5 >> outputs/ej7/rr-ej7-1-5
./simusched lotes/loteEj7.tsk 1 SchedRR 7 >> outputs/ej7/rr-ej7-1-7
./simusched lotes/loteEj7.tsk 1 SchedRR 9 >> outputs/ej7/rr-ej7-1-9
./simusched lotes/loteEj7.tsk 1 SchedRR 12 >> outputs/ej7/rr-ej7-1-12
./simusched lotes/loteEj7.tsk 1 SchedRR 15 >> outputs/ej7/rr-ej7-1-15
./simusched lotes/loteEj7.tsk 1 SchedRR 21 >> outputs/ej7/rr-ej7-1-21
./graphsched.py outputs/ej7/rr-ej7-1-2
./graphsched.py outputs/ej7/rr-ej7-1-5
./graphsched.py outputs/ej7/rr-ej7-1-7
./graphsched.py outputs/ej7/rr-ej7-1-9
./graphsched.py outputs/ej7/rr-ej7-1-12
./graphsched.py outputs/ej7/rr-ej7-1-15
./graphsched.py outputs/ej7/rr-ej7-1-21
