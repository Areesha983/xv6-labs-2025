// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#ifdef LAB_PGTBL
void superinit(void);
#endif

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
#define SUPER_POOL_N 8       // number of superpages to reserve
static char *superpool[SUPER_POOL_N];
static int superpool_count = 0;                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
#ifdef LAB_PGTBL
  superinit();
#endif
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
// Called during kinit() after free region is known.
void
superinit(void)
{
  extern char end[];  // provided by the linker (end of kernel)
  uint64 top = PHYSTOP;  // top of physical memory, defined in memlayout.h
  uint64 size = (uint64)SUPER_POOL_N * SUPERPGSIZE;

  // Align base downward to 2MB boundary
  uint64 base = (top - size) & ~(SUPERPGSIZE - 1);

  // map those pages into kernel virtual addresses and add them to pool
  for (int i = 0; i < SUPER_POOL_N; i++) {
    char *kva = (char *)P2V(base + i * SUPERPGSIZE);
    superpool[i] = kva;
  }

  superpool_count = SUPER_POOL_N;
}

// allocate one 2MB chunk
void *
superalloc(void)
{
  if (superpool_count == 0)
    return 0;

  void *kva = superpool[--superpool_count];
  superpool[superpool_count] = 0;

  // zero the memory
  memset(kva, 0, SUPERPGSIZE);
  return kva;
}

// return one 2MB chunk
void
superfree(void *kva)
{
  if (superpool_count >= SUPER_POOL_N) {
    panic("superfree: pool overflow");
  }
  superpool[superpool_count++] = kva;
}

