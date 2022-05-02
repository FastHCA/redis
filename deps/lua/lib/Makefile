
CC = g++
AR = ar rcu
RM = rm -f

BIGDECIMAL_SOURCES=bigdecimal.cpp
BIGDECIMAL_OBJS=bigdecimal.o
BIGDECIMAL_STATIC_LIB=libbigdecimal.a

all: bigdecimal

bigdecimal:
	cd bigdecimal \
		&& $(CC) -c $(BIGDECIMAL_SOURCES) \
		&& $(AR) $(BIGDECIMAL_STATIC_LIB) $(BIGDECIMAL_OBJS)

.PHONY: all bigdecimal


clean: .bigdecimal-clean

.bigdecimal-clean:
	cd bigdecimal \
		&& $(RM) $(BIGDECIMAL_STATIC_LIB) $(BIGDECIMAL_OBJS)

.PHONY: clean .bigdecimal-clean
