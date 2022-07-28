all: confmain

confmain:
	$(CC) -O2 -std=gnu11 -o $@ conf-main.c

run: confmain
	./confmain

clean:
	rm -f confmain

