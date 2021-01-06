CC = gcc
CFLAGS = -pthread

all: src/main.c src/queueCondVar.c src/queueSemaphore.c
	make condvars
	make semaphore

CV_SRC = src/main.c src/queueCondVar.c
condvars: $(CV_SRC)
	$(CC) $(CV_SRC) -D USE_COND_VARS -o out/semaphore.out $(CFLAGS)

SEM_SRC = src/main.c src/queueSemaphore.c
semaphore: $(SEM_SRC)
	$(CC) $(SEM_SRC)  -o out/condvars.out $(CFLAGS)

clean:
	rm out/*.out
	