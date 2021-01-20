## Aufgaben

1. Was ist bei der Implementierung und beim Test von Kernelmodulen anders als bei den Programmen, die bei den ersten Aufgaben zu erstellen waren? 
    - Kernel mode instead of user mode
        - Can't link against user mode headers
    - Creation of a module [23]
    - Stack size is very limited [22]
    - Code needs to be recompiled for every kernel version [26]
        - You can check the current version with macro
    - Can be optimized for a specific architecture or CPU [27]
2. Was sind Major- und Minor-Nummern und wo würden die in einer Implementierung auftauchen?
3. Wie führen Sie Tests durch? 
4. Was ist im Programmcode zu beachten?
5. Was ist andererseits in Kerneltreibern möglich?
6. Wie sieht Ihr Konzept für eine Implementierung aus
