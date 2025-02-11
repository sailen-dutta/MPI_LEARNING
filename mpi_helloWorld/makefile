EXECS=mpi_helloWorld
MPICC?=mpic++

all: ${EXECS}

mpi_helloWorld: mpi_helloWorld.cpp
	${MPICC} -o mpi_helloWorld mpi_helloWorld.cpp

clean:
	rm -f ${EXECS}
