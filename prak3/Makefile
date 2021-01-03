# Makefile for appl and mmanage
.PHONY: clean debug doc

OS	 = $(shell uname)
CC	 = /usr/bin/gcc
CFLAGS	 = -pthread -ggdb -Wall -std=gnu99 $(DFLAGS)

ifdef VMEM_PAGESIZE
# Supported page sizes 8 16 32 64
CFLAGS	+= -DVMEM_PAGESIZE=$(VMEM_PAGESIZE)
endif

ifeq ($(OS),Darwin)
	# Apple OS, librt.so not required
	LDFLAGS =  -lpthread 
else
	# Linux OS
	LDFLAGS =  -lrt -lpthread 
endif

SRCDIR   = ./src
OBJDIR   = ./obj
BINDIR   = ./bin
DOCDIR   = ./html

EXEFILES     = mmanage vmappl # Anwendungen
srcfiles     = $(wildcard $(SRCDIR)/*.c) # all src files
toolfiles    = $(patsubst %,$(SRCDIR)/%.c,$(EXEFILES))  # src files containing main
modulefiles  = $(filter-out $(toolfiles),$(srcfiles)) # modules uesd by tools; does not contain main 
deps         = $(subst $(SRCDIR)/,$(OBJDIR)/,$(srcfiles:.c=.d))

all: $(patsubst %,$(BINDIR)/%,$(EXEFILES))

debug:
	make clean
	make DFLAGS=-DDEBUG_MESSAGES

-include $(deps)

# Build all test programs

# calculate dependcies
$(OBJDIR)/%.d: $(SRCDIR)/%.c
	@echo "computing dependencies for  $<"
	@mkdir -p $(@D)
	@$(CC) -MM -MT "$@ $(patsubst %.d,%.o,$@)" -MF $@ $<

# compile a module
$(OBJDIR)/%.o: $(SRCDIR)/%.c 
	@echo "compiling $< ..."
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS)  -c $< -o $@

# link an executable
$(BINDIR)/% : $(OBJDIR)/%.o $(subst $(SRCDIR)/,$(OBJDIR)/,$(modulefiles:.c=.o)) 
	@mkdir -p $(@D)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -r -f $(OBJDIR) $(BINDIR) 
	rm -rf *.o mmanage vmappl logfile.txt pagefile.bin
	@if [ "$(OS)" != "Darwin" ]; then  rm -rf /dev/shm/sem.sem_wakeup_mmanager_vm_simulation ; fi 
	@if [ "$(OS)" != "Darwin" ]; then  rm -rf /dev/shm/sem.sem_wakeup_vmapp_vm_simulation ; fi
	@if [ "$(OS)" != "Darwin" ]; then  sudo ipcrm -ashm ; fi

doc: clean
	rm -rf $(DOCDIR)
	doxygen Doxyfile

# EOF
