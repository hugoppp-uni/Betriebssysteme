# Entwruf


## [Installation](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s02.html)
Zum Installieren werden per Bash Skript mit `sudo mknod` zwei Device Nodes erstellt.
Dafür wird die Major Nummer zuvor dynamisch ermittelt. Die Minor Nummer beträgt 0 und 1.
Außerdem wird das Gerät mit `sudo insmod` geladen.

## Ver- / Entschlüsselung
Es werden zwei Methoden geschrieben, als Parameter werden `translate_shift` und das Zeichen selber übergeben.
Es wird ein globales `char[]` verwendet, welches wie in der Aufgabenstellung definiert wird.
Zunächst wird die Position des aktuellen Zeichens im Array ausgerechnet, anschließend wird der `translate_shift` addiert, 
wobei mit dem Modulo Operator gearbeitet wird, sodass nach dem Ende letzten Index des Arrays wieder der erste folgt.
Die Entschlüsselungsmethoden kann die Verschlüsselungsmethode mit einem negiertem `translate_shift` aufrufen.

## Initialisierung und Exit
Die Init bzw Exit Methode wird beim Starten bzw. Schliesen aufgerufen. Dafür muss `module_init(<functino>)` und `module_exit(<function>)` definiert werden.
Die macros `__init` und `__exit` können ausserdem verwendet werden, um dem Kernel mitzuteilen, dass diese Methoden nicht manuel aufgerufen werden,
sodass diese nicht im Arbeitspeicher gehalten werden müssen. Bemerkenswert ist hierbei, dass der Arbeitsspeicher im Kernel-Mode nicht in die
Pagefile ausgelagert werden kann.

### Init
Mit [`kmalloc (size_t size, gfp_t flags)`](http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch11lev1sec4.html) wird Speicher für die Puffer alloziert.

Mit der Methode
```C
int __register_chrdev (	unsigned int major,
 	unsigned int baseminor,
 	unsigned int count,
 	const char * name,
 	const struct file_operations * fops);
```
wird eine major nummer registriert. (siehe [kernel.com](https://www.kernel.org/doc/htmldocs/kernel-api/API---register-chrdev.html))

Mit `device_create` wird das Gerät registriert.

### Exit

## File System
Um schreibende und lesende Funktionen zur Verfügung zu stellen, wird der Header `<linux/fs.h>` benutzt. Das struct file_operations wird wie folgt definiert: 
```c 
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
```
wobei die Werte jeweils Funktionspointer auf die in dem nächsten Abschnitten erklärten Methoden sind.

### Synchronization
Zur Synchronization wird ein Mutex verwendet. Bei Operationen auf den Puffer wird dieser gelockt.

### Open
[`int open(const char *pathname, int flags)`](https://www.man7.org/linux/man-pages/man2/open.2.html)
### Close
[`int close(int fd)`](https://www.man7.org/linux/man-pages/man2/close.2.html)
### Read
[`ssize_t read(int fd, void *buf, size_t count)`](https://man7.org/linux/man-pages/man2/read.2.html)
### Write
[`ssize_t write(int fd, const void *buf, size_t count)`](https://www.man7.org/linux/man-pages/man2/write.2.html)



## Aufgaben

1. Was ist bei der Implementierung und beim Test von Kernelmodulen anders als bei den Programmen, die bei den ersten Aufgaben zu erstellen waren? 
    - Kernel mode instead of user mode
        - higher privilege level
    - Can't link against user mode headers
    - Creation of a module [23]
    - Stack size is very limited [22]
    - Code needs to be recompiled for every kernel version [26]
        - You can check the current version with macro
    - Can be optimized for a specific architecture or CPU [27]
    - Error codes => negative integer return value as defined in <linux/errno.h> [33]
    - Normal debbugger does not work [38]
    - System can easily be corrupted -> eg crash [38]
        - Testing should be done in user space first
    - Kernel modules can not be swapped out of ram [38]
    - No floating number support
    - Manual cleaup is requierd
    - No sequential execution
2. Was sind Major- und Minor-Nummern und wo würden die in einer Implementierung auftauchen?\
     - Major
        - Identifies a device -> one major number per device
        - Dynamic alloction in startup method
     - Minor
        - One device can have multiple minor numbers
        - Ascending from 0
    - https://www.oreilly.com/openbook/linuxdrive3/book/ch03.pdf
        - The major number identifies the driver associated with the device
        - The minor number is used by the kernel to determine exactly which device is being referred to
3. Wie führen Sie Tests durch? 
    - http://derekmolloy.ie/writing-a-linux-kernel-module-part-1-introduction#Testing_the_LKM
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
    - [Bashscript](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s02.html)
        - get major version
        - create device node 2x with `sudo mknod`, use minor version 0 and 1
    - Initialisation / open
        - alloc_chrdev_region
        - Modulparameter
            - (uint) translate_bufsize = 30 
            - (uint) translate_shif = 3 
        - Allocate Buffer
    - Cleanup / release
        - Free Buffer
        - unregister_chrdev_region

