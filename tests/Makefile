vpath %.h ../include
#OBJS:=$(addprefix ../$(OBJDIR)/, multiboot.so cursor.so print.o console.o ps2.o kernel.o) 
OBJS:=$(addprefix ../$(OBJDIR)/, libc.o) 

#
tests:	stdio/itoa.c $(OBJS)  
	#$(CC) -m32 -g -I.. -o test $(filter-out ../$(OBJDIR)/multiboot.so ../$(OBJDIR)/kernel.o, $^) 
	$(CC) -m32 -I.. -g stdio/itoa.c -o test $(OBJS) 
clean:
	-rm test
