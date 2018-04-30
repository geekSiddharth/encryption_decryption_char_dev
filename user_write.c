// #include <file.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc,char **args){
    if (argc <= 1) {
        return -1;
    }

    FILE *chardev;

    chardev = fopen("/dev/chardev1", "rw");
    char ch;
    while((ch=getc(chardev))!=EOF){
        putc(ch,stdout); 
    }
    return 1;
} 