PROJECTNAME	= HA_memory
CC			= /usr/bin/gcc
CPPFLAGS	= -I include
CFLAGS 		= -W -Wall -std=gnu99 $(MODEFLAG)
LDFLAGS		= -lm
COMPILE		= $(CC) $(CFLAGS) $(CPPFLAGS)
CDIR		= source
ODIR		= object
HDIR		= include
EXE			= test
OBJ			:= $(patsubst $(CDIR)/%.c, %.o, $(wildcard $(CDIR)/*.c))
OBJFULL		:= $(addprefix $(ODIR)/, $(OBJ)) 

vpath %.c $(CDIR)
vpath %.o $(ODIR)
vpath %.h $(HDIR)

.PHONY: prod showpp open clean tar

# link
$(EXE): $(OBJ)
	@printf "\n ===> final linking to create executable $@...\n"
	$(CC) $(CFLAGS) $(OBJFULL) -o $@ $(LDFLAGS)

# compile
%.o: %.c
	@printf "\n ===> compiling object file $@...\n"
	$(COMPILE) -c $< -o $(ODIR)/$@

# header dependencies
mem_management.o: mem_management.h
test.o: mem_management.h
test2.o: mem_management.h

# debug / production differentiation
ifeq ($(MAKECMDGOALS), prod)
empty =
$(info )
$(info $(empty) ===> Building optimised production version!)
$(info $(empty) ===> Please make sure you deleted all debug object files!)
$(info )
MODEFLAG = -O3
prod: $(EXE)
else
MODEFLAG = -g3
$(OBJ): debug.h
endif

# helpers
showpp:
	$(COMPILE) $(CDIR)/*.c -E 
open:
	vi -p $(CDIR)/*.c $(HDIR)/*.h Makefile
clean:
	rm -f $(EXE) $(ODIR)/*.o $(PROJECTNAME).tar
tar:
	tar -cvf $(PROJECTNAME).tar .
