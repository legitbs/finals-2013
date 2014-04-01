all: bookworm.c bookworm-v2.c
	gcc -static -fPIC -fno-stack-protector -o bookworm bookworm.c
	strip -s bookworm
	gcc -static -fPIC -fno-stack-protector -o bookworm-v2 bookworm-v2.c
	strip -s bookworm-v2
