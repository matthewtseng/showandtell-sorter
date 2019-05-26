CC_C = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -g -pthread -std=c99 -D _POSIX_C_SOURCE=200809L -Werror
CFLAGS_NOWORKY = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all: noworky
	$(CC_C) $(CFLAGS) sorter.c network.c main.c pot.c display.c -o sorter
	cp sorter $(HOME)/cmpt433/public/myApps
	
noworky:
	$(CFLAGS_NOWORKY) noworky.c -o noworky
	cp noworky $(HOME)/cmpt433/public/myApps

clean:
	rm sorter
	rm noworky