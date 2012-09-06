FLAGS = -Wall -Wformat -std=gnu99
LIBS = -lX11
SRC_DIR = src

all: coolwm

coolwm: $(SRC_DIR)/coolwm.c
	gcc $(SRC_DIR)/coolwm.c -o coolwm  $(FLAGS) $(LIBS)

clean:
	rm -f *.o coolwm
