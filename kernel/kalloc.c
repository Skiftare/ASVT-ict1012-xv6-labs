// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
void superfreerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct superrun {
  struct superrun *next;
};

struct {
  struct spinlock lock;
  struct superrun *freelist;
  uint64 start;
  uint64 end;
} supermem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&supermem.lock, "supermem");

  //2MB superpages.
  uint64 start = PGROUNDUP((uint64)end);
  uint64 stop = (uint64)PHYSTOP;
  uint64 reserve = ((stop - start) / 4) & ~(SUPERPGSIZE - 1);
  uint64 super_start = (stop - reserve + SUPERPGSIZE - 1) & ~(SUPERPGSIZE - 1);
  uint64 super_end = stop & ~(SUPERPGSIZE - 1);

  if(super_end <= super_start){
    supermem.start = 0;
    supermem.end = 0;
    freerange((void*)start, (void*)stop);
    return;
  }

  supermem.start = super_start;
  supermem.end = super_end;

  freerange((void*)start, (void*)super_start);
  superfreerange((void*)super_start, (void*)super_end);
  freerange((void*)super_end, (void*)stop);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

void
superfreerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)(((uint64)pa_start + SUPERPGSIZE - 1) & ~(SUPERPGSIZE - 1));
  for(; p + SUPERPGSIZE <= (char*)pa_end; p += SUPERPGSIZE)
    superfree(p);
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

void
superfree(void *pa)
{
  struct superrun *r;
  uint64 addr = (uint64)pa;

  if((addr % SUPERPGSIZE) != 0 || supermem.start == 0 ||
     addr < supermem.start || addr + SUPERPGSIZE > supermem.end)
    panic("superfree");

  memset(pa, 1, SUPERPGSIZE);

  r = (struct superrun*)pa;

  acquire(&supermem.lock);
  r->next = supermem.freelist;
  supermem.freelist = r;
  release(&supermem.lock);
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

void *
superalloc(void)
{
  struct superrun *r;

  acquire(&supermem.lock);
  r = supermem.freelist;
  if(r)
    supermem.freelist = r->next;
  release(&supermem.lock);

  if(r)
    memset((char*)r, 5, SUPERPGSIZE);
  return (void*)r;
}
