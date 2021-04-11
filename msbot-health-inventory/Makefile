
INCLUDE_DIR = -I.

SOURCES = Health_Inventory_main.c \
          mytimer.c

OBJECTS := $(notdir $(SOURCES:.c=.o))

all : $(OBJECTS)
	$(CC) $(OBJECTS) -o Health_Inventory_main -lpthread -lmicrocdr -lmicroxrcedds_client

%.o : %.c
	$(CC) -g $(CFLAGS) $(INCLUDE_DIR) -o $@ -c $<
