#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int is_separator(char c) {
    const char *seps = " -\r\t\n./,";
    for(int i = 0; seps[i]; i++){
        if(c == seps[i]) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if(argc < 2){
        fprintf(2, "usage: sixfive file1 [file2 ...]\n");
        exit(1);
    }

    for(int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);
        if(fd < 0){
            fprintf(2, "sixfive: cannot open %s\n", argv[i]);
            continue;
        }
        char buf[512];
        int n;
        // Use a buffer; track building up digits
        while((n = read(fd, buf, sizeof(buf))) > 0){
            int j = 0;
            while(j < n){
                // skip separators
                while(j < n && is_separator(buf[j])) j++;
                // start of potential number
                if(j >= n) break;
                int start = j;
                while(j < n && !is_separator(buf[j])) j++;
                // now buf[start..j-1] is a token
                // check if all digits
                int all_digits = 1;
                for(int k = start; k < j; k++){
                    if(buf[k] < '0' || buf[k] > '9') {
                        all_digits = 0;
                        break;
                    }
                }
                if(all_digits && (j - start) > 0) {
                    // convert string to int
                    char numbuf[32];
                    int len = j - start;
                    if(len >= sizeof(numbuf)) len = sizeof(numbuf)-1;
                    memmove(numbuf, &buf[start], len);
                    numbuf[len] = '\0';
                    int x = atoi(numbuf);
                    if(x % 5 == 0 || x % 6 == 0){
                        printf("%d\n", x);
                    }
                }
                // continue
            }
        }
        close(fd);
    }
    exit(0);
}
