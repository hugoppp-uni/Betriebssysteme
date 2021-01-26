# Entwruf


## [Installation](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s02.html)
Zum Installieren werden per Bash Skript mit `sudo mknod` zwei Device Nodes erstellt.
Dafür wird die Major Nummer zuvor mithilfe von `awk` dynamisch ermittelt. Die Minor Nummer beträgt 0 und 1.
Außerdem wird das Gerät mit `sudo insmod` geladen.

## Ver- / Entschlüsselung
Es werden zwei Methoden geschrieben, als Parameter werden `translate_shift` und das Zeichen selber übergeben.
Es wird ein globales `char[]` verwendet, welches wie in der Aufgabenstellung definiert wird.
Zunächst wird die Position des aktuellen Zeichens im Array ausgerechnet, anschließend wird der `translate_shift` addiert, 
wobei mit dem Modulo Operator gearbeitet wird, sodass nach dem Ende letzten Index des Arrays wieder der erste folgt.
Die Entschlüsselungsmethoden kann die Verschlüsselungsmethode mit einem negiertem `translate_shift` aufrufen.

## Initialisierung und Exit
Die Init bzw Exit Methode wird beim Starten bzw. Schließen aufgerufen. Dafür muss `module_init(<functino>)` und `module_exit(<function>)` definiert werden.
Die macros `__init` und `__exit` können außerdem verwendet werden, um dem Kernel mitzuteilen, dass diese Methoden nicht manuel aufgerufen werden,
sodass diese nicht im Arbeitspeicher gehalten werden müssen. Bemerkenswert ist hierbei, dass der Arbeitsspeicher im Kernel-Mode nicht in die
Pagefile ausgelagert werden kann.

### Init
Mit [`void * kmalloc (size_t size, gfp_t flags)`](http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch11lev1sec4.html) wird Speicher für die Puffer alloziert.

Mit der Methode
```c
int __register_chrdev (	unsigned int major,
 	                    unsigned int baseminor,
                    	unsigned int count,
 	                    const char * name,
 	                    const struct file_operations * fops);
```
wird eine Major Nummer erstellt und das Gerät registriert. (siehe [kernel.com](https://www.kernel.org/doc/htmldocs/kernel-api/API---register-chrdev.html))
Dabei wird als `major` parameter 0 übergeben, sodass der Rückgabewert der Major Nummer entspricht.

### Exit
Das Gerät wird mit der `int unregister_chrdev(unsigned int major, const char *name)` Methode entfernt.

Außerdem wird der allozierte Speicher mit `void kfree (const void * objp)` wieder freigegeben. Dabei ist `objp` jener Pointer, der beim
aufrufen von `kmalloc` zurückgegeben wurde.

## [File System](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s03.html)
Um schreibende und lesende Funktionen zur Verfügung zu stellen, wird der Header `<linux/fs.h>` benutzt. 

- file
    - represents an open file descriptor
    - has fops pointer
    - points to n inode
        - there can be multiple files for one inode
- inode
    - represents a file
    - has `i_rdev` / device number
    - has fops pointer
- fops
    - holds function pointers to the operations to read / write etc.

Das struct file_operations wird wie folgt definiert: 
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

### Lesen des Modus
`MINOR(kdev_t dev)` wird verwendet, um den aktuellen Modus auszulesen. 0 bedeutet Ver-, 1 bedeutet Entschlüsselung.

### Open
`int (*open) (struct inode *, struct file *)`
Die Minor-Nummer kann mit `MINOR(inode->i_rdev)` ausgelesen werden.
### Close
`int (*release) (struct inode *, struct file *)`
### Read
`ssize_t (*read) (struct file *, char *, size_t, loff_t *)`
### Write
`ssize_t (*write) (struct file *, const char *, size_t, loff_t *)`



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

