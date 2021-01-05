CC = gcc
OBJ = main.o queueCondVar.o queueSemaphore.o

conProd: $(OBJ)
	$(CC) -o $@ $(OBJ)

main.o: main.c queueSemaphore.h
	$(CC) -c main.c

queueCondVar.o: queueCondVar.c queueCondVar.h
	$(CC) -c queueCondVar.c
	
queueSemaphore.o: queueSemaphore.c queueSemaphore.h
	$(CC) -c queueSemaphore.c

clean:
	rm conProd $(OBJ)
	