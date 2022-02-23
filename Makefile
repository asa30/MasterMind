# Build and run the MasterMind program (implemented just in C, no Assembler component)

# default goal
all: cw2

# clean up by deleting binaries
clean:
	rm cw2

# how to run the program, in debugging setup, showing the secret sequence as well
run: cw2
	./cw2 -d

# do unit testing on the matching function
unit: cw2
	sh ./test.sh

# build the app
cw2: master-mind.c
	gcc -o cw2  master-mind.c
