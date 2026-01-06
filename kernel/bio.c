// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

// 将不同的块哈希到不同的桶中，不同的桶使用不同的锁，可以减少锁竞争，提高并发性能
//
// buckets[0] --> buf[x-1] --> buf[x-2] --> ... --> buf[1] --> buf[0] --> 0
// buckets[1] --> buf[y-1] --> buf[y-2] --> ... --> buf[1] --> buf[0] --> 0
// ...
// buckets[NBUCKET-1] --> buf[z-1] --> buf[z-2] --> ... --> buf[1] --> buf[0] --> 0
//
#define NBUCKET                   13 // 桶数
#define BUCKET_HASH(dev, blockno) (((dev) + (blockno)) % NBUCKET)
struct {
  struct spinlock lock; // 只能保证同一时间只有一个线程修改 buf[i] 中的数据，不能保证链表修改的原子性
  struct buf buf[NBUF];

  struct spinlock bucket_locks[NBUCKET]; // 只能保证同一时间只有一个线程修改 buckets[i] 链表，不能保证 buf[i] 的原子性
  struct buf buckets[NBUCKET];
} bcache; // 定义变量 bcache

void
binit(void)
{
  for (int i = 0; i < NBUCKET; i++) {
    initlock(&bcache.bucket_locks[i], "bucket");
    bcache.buckets[i].next = 0;
  }

  // Initialize the free list.
  // bcache.buckets[0].next --> buf[NBUF-1] --> buf[NBUF-2] --> ... --> buf[1] --> buf[0] --> 0
  for (int i = 0; i < NBUF; i++) {
    struct buf *b = &bcache.buf[i];
    initsleeplock(&b->lock, "buffer");
    b->lastuse = 0;
    b->refcnt = 0;
    b->next = bcache.buckets[0].next;
    bcache.buckets[0].next = b;
  }

  initlock(&bcache.lock, "bcache");
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  uint key = BUCKET_HASH(dev, blockno);

  // 检查桶内是否存在相同设备和块号的缓存块，检查的时候不能让其他线程修改桶内链表，所以需要加锁
  acquire(&bcache.bucket_locks[key]);
  // bcache.buckets[0].next --> buf[NBUF-1] --> buf[NBUF-2] --> ... --> buf[1] --> buf[0] --> 0
  // bcache.buckets[key].next 是链表头
  for (b = bcache.buckets[key].next; b; b = b->next) {
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.bucket_locks[key]);
      acquiresleep(&b->lock); // 为什么要获取 b->lock？
      return b;
    }
  }

  // Not cached.

  release(&bcache.bucket_locks[key]); // 先释放桶锁，防止查找驱逐时出现环路死锁
  acquire(&bcache.lock);              // 获取驱逐锁，防止多个 CPU 同时驱逐影响后续判断
  for (b = bcache.buckets[key].next; b; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      acquire(&bcache.bucket_locks[key]);
      b->refcnt++;
      release(&bcache.bucket_locks[key]);
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Still not cached

  struct buf *before_least = 0;
  uint holding_bucket = -1;
  for (int i = 0; i < NBUCKET; i++) {
    acquire(&bcache.bucket_locks[i]);
    int newfound = 0;
    for (b = &bcache.buckets[i]; b->next; b = b->next) {
      if (b->next->refcnt == 0 && (!before_least || b->next->lastuse < before_least->lastuse)) {
        before_least = b;
        newfound = 1;
      }
    }
    if (!newfound) {
      release(&bcache.bucket_locks[i]);
    } else {
      if (holding_bucket != -1) {
        release(&bcache.bucket_locks[holding_bucket]);
      }
      holding_bucket = i;
    }
  }

  if (!before_least) {
    panic("bget: no buffers");
  }

  // Evict the least recently used (LRU) cached buffer.
  b = before_least->next;
  if (holding_bucket != key) {
    before_least->next = b->next;
    release(&bcache.bucket_locks[holding_bucket]);
    acquire(&bcache.bucket_locks[key]);
    b->next = bcache.buckets[key].next;
    bcache.buckets[key].next = b;
  }

  b->dev = dev;
  b->blockno = blockno;
  b->refcnt = 1;
  b->valid = 0;
  release(&bcache.bucket_locks[key]);
  release(&bcache.lock);
  acquiresleep(&b->lock);
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  uint key = BUCKET_HASH(b->dev, b->blockno);

  acquire(&bcache.bucket_locks[key]);
  b->refcnt--;
  if (b->refcnt == 0) {
    b->lastuse = ticks;
  }

  release(&bcache.bucket_locks[key]);
}

void
bpin(struct buf *b) {
  uint key = BUCKET_HASH(b->dev, b->blockno);

  acquire(&bcache.bucket_locks[key]);
  b->refcnt++;
  release(&bcache.bucket_locks[key]);
}

void
bunpin(struct buf *b) {
  uint key = BUCKET_HASH(b->dev, b->blockno);

  acquire(&bcache.bucket_locks[key]);
  b->refcnt--;
  release(&bcache.bucket_locks[key]);
}


