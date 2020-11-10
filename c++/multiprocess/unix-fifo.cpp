#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {

    mkfifo("/home/shtrikh17/in.fifo", 0666);
    mkfifo("/home/shtrikh17/out.fifo", 0666);
    int in = open("/home/shtrikh17/in.fifo", O_RDONLY);
    int out = open("/home/shtrikh17/out.fifo", O_WRONLY);

    char Buffer[1024];

    while(1){
        int N = read(in, Buffer, 1024);
        write(out, Buffer, N);
    }
    close(in);
    close(out);

    return 0;
}
