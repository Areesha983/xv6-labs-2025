#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void memdump(char *fmt, char *data);

int
main(int argc, char *argv[])
{
  if(argc == 1){
    printf("Example 1:\n");
    int a[2] = { 61810, 2025 };
    memdump("ii", (char*) a);
    
    printf("Example 2:\n");
    memdump("S", "a string");
    
    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *) &s);

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;
    
    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");
    
    printf("Example 4:\n");
    memdump("pihcS", (char*) &example);
    
    printf("Example 5:\n");
    memdump("sccccc", (char*) &example);
  } else if(argc == 2){
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while(n < sizeof(data)){
      int nn = read(0, data + n, sizeof(data) - n);
      if(nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}

void
memdump(char *fmt, char *data)
{
  // Your code her{
  char *p = data;

  for (int i = 0; fmt[i] != '\0'; i++) {
    char f = fmt[i];

    switch (f) {
    case 'i': { // 4-byte int
      int val = *(int*)p;
      printf("%d\n", val);
      p += sizeof(int);
      break;
    }
    case 'p': {
      uint64 val = *(uint64*)p;
      printf("%lx\n", (unsigned long)val);   // <-- fixed here
      p += sizeof(uint64);
      break;
    }
    case 'h': { // 2-byte short
      short val = *(short*)p;
      printf("%d\n", val);
      p += sizeof(short);
      break;
    }
    case 'c': { // 1-byte char
      char val = *p;
      printf("%c\n", val);
      p += 1;
      break;
    }
    case 's': { // pointer to string
      char *str = *(char**)p;      // read a char* from memory
      printf("%s\n", str);
      p += sizeof(char*);
      break;
    }
    case 'S': { // inline string
      printf("%s\n", p);
      p += strlen(p) + 1;
      break;
    }
    default:
      printf("Unknown format: %c\n", f);
      break;
    }
  }
}

