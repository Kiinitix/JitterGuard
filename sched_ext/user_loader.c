#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct bpf_object *obj;
    int err;

    obj = bpf_object__open_file("isolation.bpf.o", NULL);
    if (!obj) {
        perror("bpf_object__open_file");
        return 1;
    }

    err = bpf_object__load(obj);
    if (err) {
        fprintf(stderr, "Failed to load BPF object\n");
        return 1;
    }

    printf("Isolation scheduler loaded.\n");
    pause();

    bpf_object__close(obj);
    return 0;
}
