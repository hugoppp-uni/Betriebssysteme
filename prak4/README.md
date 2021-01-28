# Entwurf

## [Installation](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s02.html)
Zum Installieren werden per Bash Skript mit `sudo mknod` zwei Device Nodes erstellt.
Dafür wird die Major Nummer zuvor mithilfe von `awk` dynamisch ermittelt. Die Minor Nummer beträgt 0 und 1.
Außerdem wird das Gerät mit `sudo insmod` geladen.


## [Modulparameter](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch02.html#linuxdrive3-CHP-2-SECT-8)
Modulparameter werden verwendet, um die Buffergröße und die Anzahl an Zeichen, mit denen verschlüsselt werden soll, festzulegen.

Diese können wie folgt definiert werden:
```c
static char *whom = "world";
static int howmany = 1;
module_param(howmany, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);
```
Beim Laden des Modules werden diese dann mit Werten gefüllt:
```bash
insmod hellop howmany=10 whom="Mom"
```

## Ver- / Entschlüsselung
Es wird ein globales `char[]` verwendet, welches wie in der Aufgabenstellung definiert wird.
Zunächst wird die Position des aktuellen Zeichens im Array ausgerechnet, anschließend wird der `translate_shift` addiert, 
wobei mit dem Modulo Operator gearbeitet wird, sodass nach dem Ende letzten Index des Arrays wieder der erste folgt.
Im Entschlüsselungsmodus wird ein negierter `translate_shift` verwendet, somit ist keine Anpassung der Methode nötig.

## [Initialisierung und Exit](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch03.html#linuxdrive3-CHP-3-SECT-4)
Die Init bzw Exit Methode wird beim Starten bzw. Schließen aufgerufen.
Dafür muss `module_init(<function>)` und `module_exit(<function>)` definiert werden.
Die macros `__init` und `__exit` können außerdem verwendet werden, um dem Kernel mitzuteilen, dass diese Methoden nicht manuel aufgerufen werden,
sodass diese nicht im Arbeitspeicher gehalten werden müssen. Bemerkenswert ist hierbei, dass der Arbeitsspeicher im Kernel-Mode nicht in die
Pagefile ausgelagert werden kann.

```c
// Allocating Major version
int alloc_chrdev_region(dev_t *dev, 
                        unsigned int firstminor, 
                        unsigned int count, 
                        char *name);

// Initilizing the device
void cdev_init(struct cdev *cdev, struct file_operations *fops);      
int cdev_add(struct cdev *dev, dev_t num, unsigned int count);
void cdev_del(struct cdev *dev);   
int unregister_chrdev(unsigned int major, const char *name);
```

### Init
Mit der Methode `alloc_chrdev_region` wird eine Major Nummer erstellt.

Mit `cdev_init` werden zwei cdev Struktur initialisiert, die jeweils in ein `struct translate_dev` eingebettet werden.
Auf diese Struktur kann später in der open und close Methode mithilfe von `container_of` zugegriffen werden.

```c
//Geräte-Struct
struct translate_dev{
	struct cdev chardevice;
	unsigned int *buffer;
    unsigned int buffer_count;
	unsigned int *p_write;
	unsigned int *p_read;
	int shiftcount;
    char is_open_read;
    char is_open_write;
	wait_queue_head_t waitqueue_read;
	wait_queue_head_t waitqueue_write;
	struct semaphore my_semaphore;
};
```
Beim Gerät mit der Minor nummer 1 wird `translate_shift` auf den negative Wert gesetzt.
Das Buffer der Geräte wird jeweils mit 
[`void * kmalloc (size_t size, gfp_t flags)`](http://books.gigatux.nl/mirror/kerneldevelopment/0672327201/ch11lev1sec4.html) 
alloziert. 

Am Ende werden die Geräte mit `cdev_add` registiert.

Wird die Speicherallokation oder Registrierung aus irgandwelchen Gründen fehlschlagen, so wird eine etsprechende Aktion ausgefüht(z.B. wird das Gerät unregistriert)
und passende error aus `<linux/errno.h>` zurückgegeben. 

### Exit
Das Gerät wird mit der `int unregister_chrdev` Methode entfernt.

Außerdem wird der allozierte Speicher mit `void kfree (const void * objp)` wieder freigegeben. Dabei ist `objp` jener Pointer, der beim
aufrufen von `kmalloc` zurückgegeben wurde.

## [File System](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s03.html)
Um schreibende und lesende Funktionen zur Verfügung zu stellen, wird der Header `<linux/fs.h>` benutzt. 

- inode
    - representiert eine Datei
    - hat `i_rdev`
- [file](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s04.html)
    - representiert eine geöffnete Datei
    - Hat einen `file_operations` pointer
    - Zeigt auf eine `inode`
        - Es kann mehrere `file`s für eine `inode` geben
    - Hat `mode_t f_mode`, welcher eines oder beide der folgenden Bits enthalten kann:
        - `FMODE_READ`, `FMODE_WRITE`
- [file_operations](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s03.html)
    - Hält Funktionspointer, die Operationen wie Lesen / Schreiben definieren.

Das struct file_operations wird wie folgt definiert: 
```c 
static struct file_operations fops =
{
   .owner = THIS_MODULE,
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};
```
wobei die Werte jeweils Funktionspointer auf die in dem nächsten Abschnitten erklärten Methoden sind.


### [Open / Close](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s05.html)
Die Prototypen für die Open / Close Operationen sehen wie folgt aus:
```c
int (*open) (struct inode *, struct file *)
int (*release) (struct inode *, struct file *)
```
#### Open
Um auf das struct zuzugreifen, welches die nötigen Informationen hält, wird folgender Code benutzt:
```c
struct custom_struct *dev; 
dev = container_of(inode->i_cdev, struct custom_struct, cdev);
filp->private_data = dev; /* for quick access in read and write methods */
```
filp ist dabei das von der open Methode übergebene `struct file`. 
Dessen `private_data` Feld ermöglicht Zugriff auf unser Geräte struct
in den read und write Methoden.

Über `file->f_mode` wird der Modus ausgelesen.
Falls das Gerät bereits im angefordertem Modus verwendet wird, was mithilfe 
`translate_dev->is_open_read` und `translate_dev->is_open_write` abgefragt wird,
gibt die Methode `EBUSY` zurück. Andernfalls werden die Flags in dem struct entsprechended gesetzt.
Dieser Vorgang ist ein kritischer Bereich, da bei gleichzeitiger Ausführung möglicherweise mehreren 
Aufrufern Zugriff gewährt wird. Somit wird zuvor der Semaphore mit `down_interruptible` gelockt, anschließend
mit `up` wieder freigegeben
(Siehe Abschnitt [Synchronization](#synchronization)).

### [Read / Write](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/ch03s08.html)
Die Prototypen für die Read / Write Operationen sehen wie folgt aus:
```c
ssize_t (*read) (struct file *, char *, size_t, loff_t *)
ssize_t (*write) (struct file *, const char *, size_t, loff_t *)
```
der `char*` ist dabei jeweils ein Pointer auf ein buffer im User-Space, der dritte parameter ist die Größe dieses Buffers.

Zum kopieren von Daten aus / in den User-Space werden diese Funktionen verwendet:
```c
 unsigned long copy_from_user(void *to, const void *from, unsigned long count);
 unsigned long copy_to_user(void *to, const void *from, unsigned long count);
```

#### Read
Beim Lesen werden die angeforderte Anahl an Zeichen mithilfe von `copy_to_user` aus dem Buffer 
an die entsprechende Adresse im User-Space geschrieben.
`translate_dev->p_read` wird um die Anzahl an gelesener Zeichen inkrementiert, `translate_dev->buffer_count` dekrementiert.

![Read](https://www.oreilly.com/library/view/linux-device-drivers/0596000081/tagoreillycom20070220oreillyimages66866.png)

#### Write
Beim Schreiben werden die übergebenen Charactere mithilfe von `copy_from_user` in das Buffer geschrieben,
nachdem sie ver- / entschlüsselt wurden.
`translate_dev->p_write` und `translate_dev->buffer_count` werden um die Anzahl an gelesener Zeichen inkrementiert.

### [Synchronization](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch05.html)
Zur Synchronization werden pro Gerät ein Semaphore im Mutex-Modus und zwei `wait_queue`s verwendet.

#### Semaphore
```c
void init_MUTEX(struct semaphore *sem);
// lock mutex. If retval non-zero, interrupt occured => return -ERESTARTSYS
int down_interruptible(struct semaphore *sem); 
// unlock mutex
void up(struct semaphore *sem); 
```

#### Wait Queue
Alle auf das Lesen wartenden Prozesse werden in einer Queue geblockt, wenn im Buffer keine Elemente mehr vorhanden sind `translate_dev->size == 0`, 
alle auf das Schreiben wartenden Prozesse in einer anderen Queue, wenn der Buffer voll ist.
Die Wait Queue wird folgendermaßen initialisiert:

```c
init_waitqueue_head(&translate_dev->wait_queue_read);
init_waitqueue_head(&translate_dev->wait_queue_write);
```
Das Blocken kann mit der methode realisert werden:

`wait_event_interruptible(queue, condition);`

Sobald im Buffer Elemente auftauchen, kann der erste Leser aus der Queue auf diese 
Daten zugreifen. Für das Schreiben gilt das analog:

`void wake_up_interruptible(wait_queue_head_t *queue);`

## Notizen

1. Was ist bei der Implementierung und beim Test von Kernelmodulen anders als bei den Programmen, die bei den ersten Aufgaben zu erstellen waren? 
    - Kernel mode instead of user mode
        - higher privilege level
    - Can't link against user mode headers
    - A module is linked only to the kernel, and the only functions it can call are theones exported by the kernel,
    there are no libraries to link to.
    - Creation of a module [23]
    - Stack size is very limited [22]
    - Code needs to be recompiled for every kernel version [26]
        - You can check the current version with macro
    - Can be optimized for a specific architecture or CPU [27]
    - Error codes => negative integer return value as defined in <linux/errno.h> [33]
    - Normal debbugger does not work [38]
    - System can easily be corrupted -> eg crash [38]
    - Kernel modules can not be swapped out of ram [38] ???
    - Testing should be done in user space first
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
        - Before a user-space  program can access one of those device  numbers, our driver needs to connect them to its internal functions  that  implement  the  device’s  operations
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

