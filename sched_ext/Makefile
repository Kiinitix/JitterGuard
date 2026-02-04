CLANG=clang
BPF_CFLAGS=-O2 -g -target bpf -D__TARGET_ARCH_x86
USER_CFLAGS=-O2 -Wall
LIBS=-lbpf

all: isolation.bpf.o user_loader

vmlinux.h:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

isolation.bpf.o: isolation.bpf.c isolation.h vmlinux.h
	$(CLANG) $(BPF_CFLAGS) -c isolation.bpf.c -o isolation.bpf.o

user_loader: user_loader.c
	gcc $(USER_CFLAGS) user_loader.c -o user_loader $(LIBS)

clean:
	rm -f *.o user_loader
