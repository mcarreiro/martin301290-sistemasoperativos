#!/bin/bash
rm -rf outputs/ej8/*
./simusched lotes/loteEj8.tsk 1 SchedLottery 1 3 >> outputs/ej8/sl-ej8-1-3
./simusched lotes/loteEj8.tsk 1 SchedLottery 5 3 >> outputs/ej8/sl-ej8-5-2
./simusched lotes/loteEj8.tsk 1 SchedLottery 3 3 >> outputs/ej8/sl-ej8-3-5
./simusched lotes/loteEj8.tsk 1 SchedLottery 4 3 >> outputs/ej8/sl-ej8-4-7
#IO
#./simusched lotes/loteEj8IO.tsk 1 SchedLottery 1 8 >> outputs/ej8/sl-ej8-1-8
./simusched lotes/loteEj8.tsk 1 SchedLottery 2 3 >> outputs/ej8/sl-ej8-2-12
./graphsched.py outputs/ej8/sl-ej8-1-3
./graphsched.py outputs/ej8/sl-ej8-5-2
./graphsched.py outputs/ej8/sl-ej8-3-5
./graphsched.py outputs/ej8/sl-ej8-4-7
#IO
#./graphsched.py outputs/ej8/sl-ej8-1-8
./graphsched.py outputs/ej8/sl-ej8-2-12
