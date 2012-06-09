#!/bin/bash
rm -rf outputs/ej8/*
./simusched lotes/loteEj8.tsk 1 SchedLottery 1 3 >> outputs/ej8/sl-ej8-1-3
#./simusched lotes/loteEj8.tsk 1 SchedLottery 2 6 >> outputs/ej8/sl-ej8-2-6
./simusched lotes/loteEj8.tsk 1 SchedLottery 5 2 >> outputs/ej8/sl-ej8-5-2
./simusched lotes/loteEj8.tsk 1 SchedLottery 3 5 >> outputs/ej8/sl-ej8-3-5
./simusched lotes/loteEj8.tsk 1 SchedLottery 4 7 >> outputs/ej8/sl-ej8-4-7
#./simusched lotes/loteEj8.tsk 1 SchedLottery 1 8 >> outputs/ej8/sl-ej8-1-8
./simusched lotes/loteEj8.tsk 1 SchedLottery 2 12 >> outputs/ej8/sl-ej8-2-12
./graphsched.py outputs/ej8/sl-ej8-1-3
#./graphsched.py outputs/ej8/sl-ej8-2-6
./graphsched.py outputs/ej8/sl-ej8-5-2
./graphsched.py outputs/ej8/sl-ej8-3-5
./graphsched.py outputs/ej8/sl-ej8-4-7
#./graphsched.py outputs/ej8/sl-ej8-1-8
./graphsched.py outputs/ej8/sl-ej8-2-12
