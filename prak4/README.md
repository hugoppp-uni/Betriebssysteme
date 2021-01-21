## Aufgaben

1. Was ist bei der Implementierung und beim Test von Kernelmodulen anders als bei den Programmen, die bei den ersten Aufgaben zu erstellen waren? 
    - Kernel mode instead of user mode
    - Can't link against user mode headers
    - Creation of a module [23]
    - Stack size is very limited [22]
    - Code needs to be recompiled for every kernel version [26]
        - You can check the current version with macro
    - Can be optimized for a specific architecture or CPU [27]
    - Error codes => negative integer return value as defined in <linux/errno.h> [33]
    - Normal debbugger does not work [38]
    - System can easily be corrupted -> eg crash [38]
    - Kernel modules can not be swapped out of ram [38]
2. Was sind Major- und Minor-Nummern und wo würden die in einer Implementierung auftauchen?\
3. Wie führen Sie Tests durch? 
4. Was ist im Programmcode zu beachten?
    - Always check error codes
    - Init method [32]
        - If calls fail, consider exitig (first clean up)
        - When registration fail -> manually unregister
            - Kernel does not keep track of registrations
        - Kernel can access the module before initialisation is completed [35]
    - Cleanup method [34]
        - unregister and clean up
5. Was ist andererseits in Kerneltreibern möglich?
    - Module parameter [35]
    - Interrupts [38]
    - Direct memory acess [38]
    - Faster response time [38]
         - Code won't be swapped out of RAM
6. Wie sieht Ihr Konzept für eine Implementierung aus

