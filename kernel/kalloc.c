// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define PA2PGREF_ID(p)     (((p) - KERNBASE) / PGSIZE)
#define PGREF_MAX_ENTRIES  PA2PGREF_ID(PHYSTOP)

struct spinlock pgreflock;
int pgref[PGREF_MAX_ENTRIES];                               // 从 KERNBASE 开始到 PHYSTOP 物理页引用计数表
#define PA2PGREF(pa)       pgref[PA2PGREF_ID((uint64)(pa))] // 根据物理地址获取引用计数

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

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
  initlock(&pgreflock, "pgreflock");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // 当前物理页还有引用时，减少引用计数后直接返回
  acquire(&pgreflock);
  PA2PGREF(pa)--;
  if (PA2PGREF(pa) > 0) {
    release(&pgreflock);
    return;
  }
  release(&pgreflock);

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

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    PA2PGREF(r) = 1; // 新分配的物理页引用计数置为 1
  }

  return (void*)r;
}

// 当引用计数已经小于等于 1 时，不创建新引用，直接返回原物理页地址
void*
krefdec(void* pa) {
  acquire(&pgreflock);
  if (PA2PGREF(pa) <= 1) {
    release(&pgreflock);
    return pa;
  }

  // 分配新的物理页并复制内容
  uint64 newpa = (uint64)kalloc();
  if (newpa == 0) {
    release(&pgreflock);
    return 0;
  }
  memmove((void*)newpa, pa, PGSIZE);

  PA2PGREF(pa)--; // 减少原物理页引用计数

  release(&pgreflock);
  return (void*)newpa;
}

// 为 pa 增加一个引用计数
void
krefinc(void* pa)
{
  acquire(&pgreflock);
  PA2PGREF(pa)++;
  release(&pgreflock);
}

