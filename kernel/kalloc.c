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

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU]; // one for each CPU.

char* kmem_lock_names[] = {
  "kmem_0",
  "kmem_1",
  "kmem_2",
  "kmem_3",
  "kmem_4",
  "kmem_5",
  "kmem_6",
  "kmem_7",
};

void
kinit()
{
  for (int i = 0; i < NCPU; i++) {
    initlock(&kmem[i].lock, kmem_lock_names[i]);
  }
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

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  int cpu = cpuid();

  acquire(&kmem[cpu].lock);
  r->next = kmem[cpu].freelist;
  kmem[cpu].freelist = r;
  release(&kmem[cpu].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off(); // 关闭中断，防止在获取锁和释放锁之间发生中断

  int cpu = cpuid();

  acquire(&kmem[cpu].lock);

  if (!kmem[cpu].freelist) { // 当前 CPU 没有空闲内存块时，尝试从其他 CPU 偷一些
    int steal_left = 64;     // 每次最多偷 64 个内存块 (假定)
    for (int i = 0; i < NCPU; i++) {
      if (i == cpu) continue;
      acquire(&kmem[i].lock);
      struct run *rr = kmem[i].freelist;
      while (rr && steal_left) {
        // 将偷来的内存块插入到当前 CPU 空闲列表 kmem[cpu].freelist 的头部
        kmem[i].freelist = rr->next;
        rr->next = kmem[cpu].freelist;
        kmem[cpu].freelist = rr;

        // 移动到下一个内存块，再次进入 while 循环，检查是否需要继续偷
        rr = kmem[i].freelist;
        steal_left--;
      }
      release(&kmem[i].lock);
      if (steal_left == 0) break; // 如果已经偷够了，就退出循环
    }
  }

  r = kmem[cpu].freelist;
  if(r)
    kmem[cpu].freelist = r->next;

  release(&kmem[cpu].lock);

  pop_off(); // 恢复中断

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
