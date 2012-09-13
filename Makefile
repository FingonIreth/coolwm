FLAGS = -Wall -Wformat -std=gnu99 -g3
LIBS = -lX11 -lXinerama `pkg-config --cflags --libs glib-2.0`
SRC_DIR = src

all: coolwm

coolwm: $(SRC_DIR)/coolwm.c utils.o grab.o handlers.o
	gcc $(SRC_DIR)/coolwm.c -o coolwm utils.o grab.o handlers.o $(FLAGS) $(LIBS)

utils.o: $(SRC_DIR)/utils.c
	gcc -c $(SRC_DIR)/utils.c -o utils.o $(FLAGS) $(LIBS)

grab.o: $(SRC_DIR)/grab.c
	gcc -c $(SRC_DIR)/grab.c -o grab.o $(FLAGS) $(LIBS)

handlers.o: $(SRC_DIR)/handlers.c
	gcc -c $(SRC_DIR)/handlers.c -o handlers.o $(FLAGS) $(LIBS)

clean:
	rm -f *.o coolwm
