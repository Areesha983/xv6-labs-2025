
#include "../kernel/types.h"
#include "../kernel/stat.h"
#include "../kernel/fcntl.h"
#include "user.h"

/* is_alnum_char: return 1 if c is alphanumeric */
int is_alnum_char(char c) {
  if (c >= '0' && c <= '9') return 1;
  if (c >= 'A' && c <= 'Z') return 1;
  if (c >= 'a' && c <= 'z') return 1;
  return 0;
}

int
main(int argc, char *argv[])
{
  unsigned long p;
  char *buf;
  int n;

  printf("attack: compile & run test\n");

  /* quick scan test */
  printf("is_alnum_char('A') = %d\n", is_alnum_char('A'));
  printf("is_alnum_char('!') = %d\n", is_alnum_char('!'));

  /* example allocation using sbrk (harmless) */
  buf = (char*) sbrk(512);    /* request 512 bytes */
  if (buf == (char*) -1) {
    printf("sbrk failed\n");
    exit(1);
  }
  for (n = 0; n < 31; n++) buf[n] = 'x';
  buf[31] = '\0';

  /* Preferred: simple %s which xv6 supports */
  printf("allocated buf: %s\n", buf);

  /* Alternative: raw write (no printf formatting) */
  /* write(1, "allocated buf (raw): ", 21);
     write(1, buf, 31);
     write(1, "\n", 1);
  */

  p = (unsigned long) buf;
  if (p == 0) {
    printf("buf is null\n");
    exit(1);
  }

  exit(0);
  return 0;
}

