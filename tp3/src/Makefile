MPICC   = mpicc
MPIEXEC = mpiexec
CFLAGS += -Wall

EJECUTABLE = tp3

.PHONY : all new clean run

all : $(EJECUTABLE) run

tp3 : tp3.c srv.c
	$(MPICC) $(CFLAGS) -o tp3 tp3.c srv.c

run : tp3
	$(MPIEXEC) -np 2 ./$(EJECUTABLE)  '.' 500 500 1 500 500 >/dev/null

clean :
	rm -f $(EJECUTABLE)

new : clean all
