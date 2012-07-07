C_SRCS=$(wildcard *.c)
CPP_SRCS=$(wildcard *.cpp)

LIB_NAME=../libprophet.a

LIB_SRCS=

SUB_PROJECTS=				

INCLUDE_DIRS=					\
		.				\
		../prophet/include		\
		../LightWaveLoader		\
		/usr/local/include		\

# MAKEFILE Magic
DEPS =$(CPP_SRCS:%.cpp=%.d)
DEPS+=$(C_SRCS:%.c=%.d)

OBJS =$(CPP_SRCS:%.cpp=%.o)
OBJS+=$(C_SRCS:%.c=%.o)

INC_DIRS_WITH_I=$(INCLUDE_DIRS:%=-I%)

SUB_PROJECTS_MAKE=$(SUB_PROJECTS:%=cd %; make; cd ..;)
SUB_PROJECTS_CLEAN=$(SUB_PROJECTS:%=cd %; make clean; cd ..;)

CC=gcc
CPP=g++
LNK= $(CC)
ARLIB=ar

CPPFLAGS= -c -g
CFLAGS= -c -g
LNKFLAGS= -g

%.d : %.cpp
	$(CPP) -MM $(CPPFLAGS) $(INC_DIRS_WITH_I) -D LINUX $< >$@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d : %.c
	$(CC) -MM $(CFLAGS) $(INC_DIRS_WITH_I) -D LINUX $< >$@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
%.o : %.cpp 
	$(CPP) $(CPPFLAGS) $(INC_DIRS_WITH_I) -D LINUX $< -o $@ 

%.o : %.c
	$(CC) $(CFLAGS) $(INC_DIRS_WITH_I) -D LINUX $< -o $@ 

$(LIB_NAME): subs $(OBJS) tags $(LIB_SRCS)
	$(ARLIB) rcs $(LIB_NAME) $(OBJS) $(LIB_SRCS)

subs : 
	$(SUB_PROJECTS_MAKE)

tags: $(DEPS) $(CPP_SRC) $(C_SRC)
	ctags *

clean: 
	$(SUB_PROJECTS_CLEAN)
	rm -f $(LIB_NAME) $(OBJS) $(LIB_SRCS) $(DEPS) tags 
	
-include $(DEPS)

