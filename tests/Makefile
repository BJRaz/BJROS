vpath %.h ../include

tests:	stdio/itoa.c 
	#$(CC) -m32 -g -I. -o test $(filter-out ../$(OBJDIR)/multiboot.so ../$(OBJDIR)/kernel.o, $^) 
	$(CC) -m32 -I.. -g stdio/itoa.c -o test ../$(OBJDIR)/atoi.so ../bin/atou.so ../bin/utoa.so ../bin/itoa.so ../bin/utox.so ../bin/strlen.so ../bin/string.so 
