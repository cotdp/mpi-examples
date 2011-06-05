MPICC=/usr/lib64/openmpi/bin/mpicc

all: clean
	$(MPICC) src/hello.c -o bin/hello
	$(MPICC) src/master_servant.c -o bin/master_servant
	$(MPICC) src/master_servant2.c -o bin/master_servant2
	$(MPICC) src/bogomips.c -o bin/bogomips

clean:
	rm -f bin/*
