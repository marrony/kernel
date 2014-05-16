#include <stdint.h>
#include <string.h>

#include "asm.h"
#include "heap.h"
#include "syscall.h"
#include "interrupt.h"
#include "kernel.h"
#include "global.h" 

int fork() {
    int ret;
    __asm__ __volatile__ (
        "int $0x80" : "=a"(ret) : "a"(SYSTEM_fork)
    );
    return ret;
}

int getpid() {
    int ret;
    __asm__ __volatile__ (
        "int $0x80" : "=a"(ret) : "a"(SYSTEM_getpid)
    );
    return ret;
}

void keyboard() {
    uint8_t status = inb(0x64);
    uint8_t data = inb(0x60);
    kprintf("a key was pressed status: %d data: %d\n", status, data);
}

#define DATA      0
#define ERROR     1
#define NSECTOR   2
#define SECTOR    3
#define LBA_0     3
#define LOW_CYL   4
#define LBA_1     4
#define HIGH_CYL  5
#define LBA_2     5
#define DRIVE     6
#define LBA_3     6
#define STATUS    7
#define COMMAND   7
#define CONTROL   0x206

#define DRV_MASK  0xa0
#define CHS_MODE  (0 << 6)
#define LBA_MODE  (1 << 6)
#define MASTER    ((0 << 4) | DRV_MASK)
#define SLAVE     ((1 << 4) | DRV_MASK)

#define IDE0 0x1f0
#define IDE1 0x170

#define CMD_READ            0x20
#define CMD_WRITE           0x30
#define CMD_IDENTIFY_ATA    0xEC
#define CMD_IDENTIFY_ATAPI  0xA1

#define PATAPI 1
#define SATAPI 2
#define PATA   3
#define SATA   4

#define ST_BUSY            (1 << 7)
#define ST_READY           (1 << 6)
#define ST_DATA_REQUEST    (1 << 3)
#define ST_ERROR           (1 << 0)

typedef unsigned long offset_t;
typedef uint32_t mode_t;
typedef uint32_t dev_t;
typedef int32_t ino_t;

struct ide_device_t {
    uint16_t iobase;
    uint8_t slave;
    uint8_t present;
    uint8_t atapi;
    uint8_t lba;
    uint8_t dma;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors; 
    uint32_t capacity; 
};

static struct ide_device_t ide_devices[4] = {
    {IDE0, 0, 0},
    {IDE0, 1, 0},
    {IDE1, 0, 0},
    {IDE1, 1, 0}
};

//milli = 1 / 1.000
//micro = 1 / 1.000.000
//nano  = 1 / 1.000.000.000

void nsleep(int nano) {
    for(int i = 0; i < nano*1000; i++)
        __asm__ __volatile__ ("nop");
}

void usleep(int micro) {
}

void msleep(int milli) {
    uint32_t next = ticks + milli/(1000/HZ); 

    while(ticks < next)
        ;
}

void ide_nsleep(struct ide_device_t* device, int nano) {
    nsleep(nano);
}

int ide_device_selection(struct ide_device_t* device);

void ide_read_write(struct ide_device_t* device, offset_t offset, void* buffer, size_t size, int direction) {
    uint32_t lba = offset; 
    uint8_t sectors = size / 512;

    uint16_t iobase = device->iobase;

    ide_device_selection(device);

    outb(iobase + NSECTOR, sectors);
    outb(iobase + LBA_0, lba & 0xff);
    outb(iobase + LBA_1, (lba >> 8) & 0xff);
    outb(iobase + LBA_2, (lba >> 16) & 0xff);
    outb(iobase + LBA_3, ((lba >> 24) & 0xf) | MASTER | LBA_MODE);
    outb(iobase + COMMAND, direction ? CMD_READ : CMD_WRITE);

    nsleep(400);

    for(int i = 0; i < sectors; i++) {
        if((inb(iobase + STATUS) & ST_ERROR) != 0)
            return;

        while((inb(iobase + STATUS) & ST_BUSY) != 0)
            ;

        if(direction) {
            for(int j = 0; j < 256; j++)
                ((uint16_t*)buffer)[j] = inw(iobase + DATA);
        } else {
            for(int j = 0; j < 256; j++)
                outw(iobase + DATA, ((uint16_t*)buffer)[j]);
        }

        buffer += 512;
    }
}

int ide_read(dev_t dev, offset_t offset, void* buffer, size_t size) {
    struct ide_device_t* ide = &ide_devices[dev];

    if(!ide->present)
        return -1;

    ide_read_write(ide, offset, buffer, size, 1);
    return 0;
}

int ide_write(dev_t dev, offset_t offset, void* buffer, size_t size) {
    struct ide_device_t* ide = &ide_devices[dev];

    if(!ide->present)
        return -1;

    ide_read_write(ide, offset, buffer, size, 0);
    return 0;
}

/*
outb (0x1F2, sectorcount high)
outb (0x1F3, LBA4)
outb (0x1F4, LBA5)         -- value = 0 or 1 only
outb (0x1F5, LBA6)         -- value = 0 always
outb (0x1F2, sectorcount low)
outb (0x1F3, LBA1)
outb (0x1F4, LBA2)
outb (0x1F5, LBA3)
*/

int device_type(uint16_t ide, uint8_t slave) {
    outb(ide + DRIVE, DRV_MASK | (slave << 4));

    inb(ide + CONTROL);
    inb(ide + CONTROL);
    inb(ide + CONTROL);
    inb(ide + CONTROL);

    uint8_t lo = inb(ide + LOW_CYL);
    uint8_t hi = inb(ide + HIGH_CYL);

    if(lo == 0x69 && hi == 0x96) return SATAPI;
    if(lo == 0x3c && hi == 0xc3) return SATA;

    return -1;
}

int ide_wait(struct ide_device_t* device, uint8_t mask, uint8_t value, int timeout) {
    uint8_t status;

    do {
        status = inb(device->iobase + STATUS);
        ide_nsleep(device, 1000);
    } while((status & mask) != value && --timeout);

    return timeout;
}

void ide_reset_controller(struct ide_device_t* device) {
    outb(device->iobase + CONTROL, 0x04);

    ide_nsleep(device, 400);

    outb(device->iobase + DRIVE, DRV_MASK | (device->slave << 4));

    if(!ide_wait(device, ST_BUSY, ST_BUSY, 1)) {
    }
    
    outb(device->iobase + CONTROL, 0);

    if(!ide_wait(device, ST_BUSY | ST_READY, ST_READY, 300)) {
    }
}

int ide_device_selection(struct ide_device_t* device) {
    uint16_t iobase = device->iobase;

    if((inb(iobase + STATUS) & (ST_BUSY | ST_DATA_REQUEST)) != 0) {
        return 0;
    }

    outb(iobase + DRIVE, DRV_MASK | (device->slave << 4));

    ide_nsleep(device, 400);

    if((inb(iobase + STATUS) & (ST_BUSY | ST_DATA_REQUEST)) != 0) {
        return 0;
    }

    return 1;
}

void ide_fix_string(char* str, int len) {
    for(int i = 0; i < len; i += 2) {
        char ch = str[i];
        str[i] = str[i+1];
        str[i+1] = ch;
    }
}

void init_ide_devices() {
    for(int i = 0; i < 4; i++) {
        struct ide_device_t* device = &ide_devices[i];
        uint16_t iobase = device->iobase;

        device->present = 0;

        ide_reset_controller(device);

        if(!ide_device_selection(device))
            continue;

        //uint8_t sc = inb(iobase + NSECTOR);
        //uint8_t sn = inb(iobase + SECTOR);
        uint8_t cl = inb(iobase + LOW_CYL);
        uint8_t ch = inb(iobase + HIGH_CYL);
        uint8_t st = inb(iobase + STATUS);

        if(cl == 0x14 && ch == 0xeb) {
            device->present = 1;
            device->atapi = 1;
        }

        if(cl == 0x00 && ch == 0x00 && st != 0) {
            device->present = 1;
            device->atapi = 0;
        }

        if(!device->present)
            continue;

        outb(iobase + COMMAND, device->atapi ? CMD_IDENTIFY_ATAPI : CMD_IDENTIFY_ATA);

        ide_nsleep(device, 400);

        if(!ide_wait(device, ST_BUSY | ST_DATA_REQUEST | ST_ERROR, ST_DATA_REQUEST, 300)) {
            device->present = 0;
            continue;
        }

        uint16_t info_data[256];
        for(int j = 0; j < 256; j++)
            info_data[j] = inw(iobase + DATA);

        device->lba = (info_data[49] >> 9) & 1;
        device->dma = (info_data[49] >> 8) & 1;
        device->cylinders = info_data[1];
        device->heads = info_data[3];
        device->sectors = info_data[6];

        if(device->lba)
            memcpy(&device->capacity, &info_data[60], sizeof(uint32_t));
        else
            device->capacity = device->cylinders * device->heads * device->sectors;

#if 0
        char buffer[50];

        memcpy(buffer, &info_data[27], 40);
        ide_fix_string(buffer, 40);
        buffer[40] = 0;
        kprintf("model: %s\n", buffer);

        memcpy(buffer, &info_data[10], 20);
        ide_fix_string(buffer, 20);
        buffer[20] = 0;
        kprintf("serial: %s\n", buffer);

        memcpy(buffer, &info_data[23], 8);
        ide_fix_string(buffer, 8);
        buffer[8] = 0;
        kprintf("firmware: %s\n", buffer);

        kprintf("lba: %d, dma: %d\n", device->lba, device->dma);
        kprintf("chs: %d %d %d -> %d %d %d\n", device->cylinders, device->heads, device->sectors,
                    info_data[54], info_data[55], info_data[56]);
        kprintf("capacity: %u %d\n\n", device->capacity, info_data[57] | (info_data[58] << 16));
#endif
    }
}

#define MAX_FILESYSTEMS 4
#define MAX_BLOCKDEVICES 10
#define MAX_NODES 10
#define MAX_MOUNTPOINTS 10

struct filesystem_t {
    const char* type;

    struct super_block_t* (*get_sb)(int flags, void* data);
    void (*free_sb)(struct super_block_t* sb);
};

struct dentry_t;
struct inode_t;

struct inode_operations_t {
    int (*create)(struct inode_t* dir, struct dentry_t* dentry, int mode);
    // return NULL on success or error code
    struct dentry_t* (*lookup)(struct inode_t* dir, struct dentry_t* dentry);
    int (*mkdir)(struct inode_t* dir, struct dentry_t* dentry, int mode);
};

struct file_operations_t {
};

struct inode_t {
    ino_t ino;
    mode_t mode;
    int state;
    dev_t rdev;
    int size;
    int count;
    struct dentry_t* dentry;
    uint32_t blocksize;
    char data[1024];
    struct inode_operations_t* op;
    struct file_operations_t* fop;
    struct super_block_t* sb;
};

#define S_NEW       1

#define M_FILE      1
#define M_DIRECTORY 2

#define S_ISBLK(m)
#define S_ISCHR(m)
#define S_ISDIR(m) (((m) & M_DIRECTORY) == M_DIRECTORY)
#define S_ISFIFO(m)
#define S_ISREG(m) (((m) & M_FILE) == M_FILE)
#define S_ISLNK(m)
#define S_ISSOCK(m)

struct dentry_operations_t {
};

struct dentry_t {
    struct super_block_t* sb;
    struct inode_t* inode;
    struct dentry_t* parent;
    struct dentry_operations_t* op;
    struct dentry_t* subdirs[32];  //should be a list
    char name[32];
};

struct super_operations_t {
    struct inode_t* (*alloc_inode)(struct super_block_t* sb);
    void (*read_inode)(struct inode_t* inode);   //read inode from disk
    void (*write_inode)(struct inode_t* inode);  //write inode to disk
};

struct super_block_t {
    dev_t dev;
    struct dentry_t* root;
    uint32_t blocksize;
    struct super_operations_t* op;
    struct filesystem_t* type;
    struct block_device_t* device; 
    void* specific;

    struct inode_t* inodes[1024];
    struct dentry_t* dentries[1024];
};

struct mountpoint_t {
    struct mountpoint_t* parent;
    struct dentry_t* mountpoint;
    struct dentry_t* root;
    struct super_block_t* sb;
    struct mountpoint_t* mounts[10];
    const char* devname;
};

struct node_t {
    const char* devname;
    dev_t dev;
    mode_t mode;
};

struct block_device_t {
    dev_t dev;
    int blocksize;
    int begin;
    int end;

    int (*read)(dev_t dev, offset_t offset, void* buffer, size_t size);
    int (*write)(dev_t dev, offset_t offset, void* buffer, size_t size);
};

struct inode_t* generic_alloc_inode(struct super_block_t* sb) {
    for(int i = 0; i < 1024; i++) {
        if(sb->inodes[i])
            continue;
        sb->inodes[i] = (struct inode_t*) kmalloc(sizeof(struct inode_t));
        memset(sb->inodes[i], 0, sizeof(struct inode_t));
        sb->inodes[i]->sb = sb;
        sb->inodes[i]->state = S_NEW;
        return sb->inodes[i];
    }
    return 0;
}

struct dentry_t* _d_alloc(struct super_block_t* sb, const char* str) {
    struct dentry_t* dentry = (struct dentry_t*)kmalloc(sizeof(struct dentry_t));
    dentry->parent = 0;
    dentry->sb = sb;
    strcpy(dentry->name, str);
    return dentry;
}

struct dentry_t* d_alloc(struct dentry_t* parent, const char* str) {
    struct dentry_t* dentry = _d_alloc(parent->sb, str); 
    dentry->parent = parent;

    for(int i = 0; i < 32; i++) {
        if(!parent->subdirs[i]) {
            parent->subdirs[i] = dentry;
            break;
        }
    }

    return dentry;
}

#define __section(x) __attribute__((__section__(#x)))
#define __init __section(.init.text)

void __init test() {
}

void d_instantiate(struct dentry_t* dentry, struct inode_t* inode) {
    dentry->inode = inode;
    inode->dentry = dentry;
}

void d_add(struct dentry_t* dentry, struct inode_t* inode) {
    d_instantiate(dentry, inode);
}

#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

struct dentry_t* d_create_root(struct inode_t* inode) {
    struct dentry_t* root = _d_alloc(inode->sb, "/");

    d_add(root, inode);

    return root;
}

struct inode_t* alloc_inode(struct super_block_t* sb) {
    struct inode_t* inode = sb->op->alloc_inode(sb);
    return inode;
}

struct inode_t* iget(struct super_block_t* sb, ino_t ino) {
    for(int i = 0; i < 1024; i++) {
        if(sb->inodes[i] && sb->inodes[i]->ino == ino) {
            sb->inodes[i]->count++;
            return sb->inodes[i];
        }
    }

    struct inode_t* inode = alloc_inode(sb);

    if(!inode)
        return 0;

    inode->ino = ino;
    inode->state = S_NEW;
    return inode;
}

void iput(struct inode_t* inode) {
    for(int i = 0; i < 1024; i++) {
        if(inode->sb->inodes[i] == inode) {
            if(--inode->count == 0) {
                inode->sb->inodes[i] = 0;
                kfree(inode);
            }
        }
    }
}

void dput(struct dentry_t* dentry) {
}

/*
 sb->op->read_inode(sb->root->inode);
 sb->root->inode->op->lookup();

 /file1   -> file
 /file2   -> file
 /dev     -> directory

 inode      <---+       dentry <--+<--+<-+<-+
 type: DIR      |       parent ---+   |  |  |
 ino: 0         +-----  inode         |  |  |
 data: []               name: /       |  |  |
                        subdirs[0] -------------+
 inode      <------+    subdirs[1] ----------------+
 type: FILE        |    subdirs[2] -------------------+
 ino: 1            |                  |  |  |   |  |  |
 data: []          |    dentry  <---------------+  |  |
                   |    parent -------+  |  |      |  |
 inode      <---+  +--  inode            |  |      |  |
 type: FILE     |       name: file1      |  |      |  |
 ino: 2         |       subdirs          |  |      |  |
 data: []       |                        |  |      |  |
                |       dentry  <------------------+  |
 inode     <-+  |       parent ----------+  |         |
 type: DIR   |  +-----  inode               |         |
 ino: 3      |          name: file2         |         |
 data: []    |          subdirs[]           |         |
             |                              |         |
             |          dentry  <---------------------+
             |          parent -------------+
             +--------  inode
                        name: dev
                        subdirs[]

 */

struct filesystem_t* filesystems[MAX_FILESYSTEMS] = {0};
struct mountpoint_t mountpoints[MAX_MOUNTPOINTS] = {{0}};
struct node_t nodes[MAX_NODES] = {{0}};
struct block_device_t block_devices[MAX_BLOCKDEVICES];
size_t block_count = 0;

int register_filesystem(struct filesystem_t* filesystem) {
    for(int i = 0; i < MAX_FILESYSTEMS; i++) {
        if(!filesystems[i]) {
            filesystems[i] = filesystem;
            return 0;
        }
    }
    return -1;
}

struct filesystem_t* find_filesystem(const char* type) {
    for(int i = 0; i < MAX_FILESYSTEMS; i++) {
        if(filesystems[i] && !strcmp(type, filesystems[i]->type))
            return filesystems[i];
    }
    return 0;
}

struct node_t* find_node(const char* devname) {
    for(int i = 0; i < MAX_NODES; i++) {
        if(nodes[i].devname && !strcmp(nodes[i].devname, devname))
            return &nodes[i];
    }
    return 0;
}

int mount(const char* type, const char* dir, int flags, void* data) {
    kprintf("mount 0\n");

    const char* devname = (const char*)data;

    struct node_t* node = find_node(devname);
    if(!node)
        goto error;

    kprintf("mount 1\n");
    struct filesystem_t* filesystem = find_filesystem(type);

    if(!filesystem)
        goto error;

    kprintf("mount 2\n");
    struct super_block_t* sb = filesystem->get_sb(flags, data);

    kprintf("mount 3\n");
    if(!sb)
        goto error;

    for(int i = 0; i < MAX_MOUNTPOINTS; i++) {
        if(!mountpoints[i].mountpoint) {
            mountpoints[i].devname = node->devname;
            mountpoints[i].sb = sb;
            return 0; 
        }
    }

error:
    return -1;
}

int umount(const char* dir, int flags) {
    return -1;
}

int mknod(const char* devname, mode_t mode, dev_t dev) {
    for(int i = 0; i < MAX_NODES; i++) {
        if(!nodes[i].devname) {
            nodes[i].devname = devname;
            nodes[i].dev = dev;
            nodes[i].mode = mode;
            return 0;
        }
    }

    return -1;
}

int register_block_device(struct block_device_t* block_device) {
    if(block_count >= MAX_BLOCKDEVICES)
        return -1;

    memcpy(&block_devices[block_count], block_device, sizeof(struct block_device_t));
    block_count++;
    return 0;
}

struct block_device_t* find_block_device(dev_t dev) {
    for(int i = 0; i < block_count; i++)
        if(block_devices[i].dev == dev)
            return &block_devices[i];
    return 0;
}

int open(const char* path) {
    return -1;
}

int close(int fd) {
    return -1;
}

int read(int fd, void* buffer, size_t size) {
    return -1;
}

int write(int fd, void* buffer, size_t size) {
    return -1;
}

struct dentry_t* mount_bdev(struct filesystem_t* fs, int flags, const char* devname, void* data) {
    return 0;
}

#define NERIS_BLOCK0_OFF 2
#define NERIS_BLOCK_SIZE 1024
#define NERIS_INODES_PER_BLOCK (NERIS_BLOCK_SIZE / sizeof(struct neris_inode_t))
#define NERIS_BLOCK(x) (NERIS_BLOCK0_OFF + (x)*2)

struct neris_inode_t {
    uint8_t type;    //1 -> file 2 -> directory
    int32_t size;
    int32_t blockpointers[8];
    int8_t used;
} __attribute__((packed));

struct neris_dentry_t {
    char name[8];
    ino_t ino;
} __attribute__((packed));

struct neris_super_block_t {
    uint8_t free_list[NERIS_BLOCK_SIZE];
    union {
        uint8_t padding[NERIS_BLOCK_SIZE];
        struct neris_inode_t inode_list[NERIS_INODES_PER_BLOCK];
    };
} __attribute__((packed));

struct inode_t* neris_iget(struct super_block_t* sb, ino_t ino) {
    struct inode_t* inode = iget(sb, ino);

    if(!inode)
        return 0;

    if((inode->state & S_NEW) == 0)
        return inode;

    struct neris_super_block_t* neris_sb = sb->specific;
    struct neris_inode_t* neris_inode = &neris_sb->inode_list[ino];

    inode->mode = neris_inode->type;
    inode->state = S_NEW;
    inode->ino = ino;

    //TODO verify the type and initialize operations correctly

    return inode;
}

/*
 * inode      inode       inode       inode       dentry      dentry
 * ino: 0     ino: 1      ino: 2      ino: 3      .     | 0   .     | 3
 * mode: dir  mode: file  mode: file  mode: dir   ..    | 0   ..    | 0
 * size: 10k  size: 10k   size: 10k   size: 10k   file1 | 1
 * block: 0   block: 1    block: 2    block: 3    file2 | 2
 *                                                dir   | 3
 *
 * +----+-----------+--------------+-------------------+
 * | sb | free list |0|1|2| inodes |0|1|2| data blocks |
 * +----+-----------+--------------+-------------------+
 */

void* idata(struct inode_t* inode) {
    struct super_block_t* sb = inode->sb;
    struct neris_super_block_t* neris_sb = (struct neris_super_block_t*)sb->specific;

    if((inode->state & S_NEW) != 0) {
        int block = neris_sb->inode_list[inode->ino].blockpointers[0];
        kprintf("new inode %d\n", block);
        sb->device->read(inode->rdev, NERIS_BLOCK(block), inode->data, NERIS_BLOCK_SIZE); 
        inode->state &= ~S_NEW;
    }

    return inode->data;
}

ino_t find_inode_by_name(struct dentry_t* dentry) {
    //TODO verify that parent's inode is new and populate its subdirs
    struct inode_t* inode = dentry->parent->inode;
    struct neris_dentry_t* neris_inode = idata(inode);

    for(int i = 0; i < NERIS_INODES_PER_BLOCK; i++) {
        ino_t ino = neris_inode[i].ino;

        if(ino == 0)
            continue;

        if(!strncmp(neris_inode[i].name, dentry->name, 8))
            return ino;
    }

    return 0;
}

struct dentry_t* d_lookup(struct inode_t* dir, struct dentry_t* name) {
    struct inode_t* inode = 0;

    ino_t ino = find_inode_by_name(name);

    if(ino) {
        inode = neris_iget(dir->sb, ino);
        if(!inode)
            return (void*)-1;    
    }

    d_add(name, inode);
    return 0;
}

void neris_read_inode(struct inode_t* inode) {   //read inode from disk
    if((inode->state & S_NEW) == 0)
        return;

    if(S_ISDIR(inode->mode)) {
        struct neris_super_block_t* neris_sb = inode->sb->specific;
        struct neris_dentry_t* neris_inode = idata(inode);

        for(int i = 0; i < NERIS_INODES_PER_BLOCK; i++) {
            ino_t ino = neris_inode[i].ino;

            if(ino == 0)
                continue;

            struct dentry_t* child = d_alloc(inode->dentry, neris_inode[i].name);
            struct inode_t* new = iget(inode->sb, ino);
            new->mode = neris_sb->inode_list[ino].type;

            new->rdev = inode->rdev;

            d_add(child, new);
        }
    }
}

void neris_write_inode(struct inode_t* inode) {  //write inode to disk
}

struct super_operations_t neris_super_block_operations = {
    .alloc_inode = &generic_alloc_inode,
    .read_inode = &neris_read_inode,
    .write_inode = &neris_write_inode
};

struct inode_t* neris_root;

struct super_block_t* neris_get_sb(int flags, void* data) {
    const char* devname = (const char*)data;

    struct node_t* node = find_node(devname);
    if(!node) return 0;

    struct block_device_t* device = find_block_device(node->dev);
    if(!device) return 0;

    struct super_block_t* sb = (struct super_block_t*) kmalloc(sizeof(struct super_block_t));
    sb->op = &neris_super_block_operations;
    sb->root = d_create_root(neris_root);
    sb->dev = node->dev;
    sb->device = device;
    sb->specific = kmalloc(sizeof(struct neris_super_block_t));

    struct neris_super_block_t* neris_sb = sb->specific;
    device->read(node->dev, NERIS_BLOCK(0), neris_sb->free_list, NERIS_BLOCK_SIZE);
    device->read(node->dev, NERIS_BLOCK(1), neris_sb->inode_list, NERIS_BLOCK_SIZE);

    neris_root = sb->op->alloc_inode(sb);
    neris_root->mode = M_DIRECTORY;
    neris_root->state = S_NEW;
    neris_root->rdev = node->dev;
    neris_root->ino = 0; 

    struct dentry_t* dir = d_alloc(sb->root, "dir");
    if(!d_lookup(neris_root, dir)) {
        if(dir->inode->mode != M_DIRECTORY) {
            struct dentry_t* file = d_alloc(dir, "file3");
            if(!d_lookup(dir->inode, file)) {
                kprintf("content of file\n%s\n", idata(file->inode));
            }
        } else
            kprintf("dir should be a directory\n");
    }

    return sb;
}

void neris_free_sb(struct super_block_t* sb) {
    kfree(sb->specific);
    kfree(sb);
}

int kmain() {
    cli();
    init_descriptor();
    init_interrupt_controller();
    init_paging(32*1024*1024);
    init_tasking();
    init_system_call();
    
    set_interrupt_handler(IRQ0 + 1, &keyboard);

    init_ide_devices();

#if 0
    struct filesystem_t devfs = {
        .type = "devfs"
    };

    register_filesystem(&devfs);
    mount("devfs", "/dev/", 0, 0);

    struct filesystem_t fatfs = {
        .type = "fat"
    };

    register_filesystem(&fatfs);
#endif

    struct filesystem_t nerisfs = {
        .type = "neris",
        .get_sb = &neris_get_sb,
        .free_sb = &neris_free_sb
    };

    register_filesystem(&nerisfs);

    dev_t dev = 0;

    struct block_device_t block_device = {
        .dev = dev,
        .read = &ide_read,
        .write = &ide_write
    };

    register_block_device(&block_device); //just major
    mknod("/dev/disk0", 0, dev); //major & minor
    mount("neris", "/mnt/neris/", 0, "/dev/disk0");

#if 0
    kprintf("filesystems\n");
    for(int i = 0; i < MAX_FILESYSTEMS; i++) {
        if(filesystems[i].type)
            kprintf("name: %s\n", filesystems[i].type);
    }
    
    kprintf("\nnodes\n");
    for(int i = 0; i < MAX_NODES; i++) {
        if(nodes[i].devname)
            kprintf("name: %s dev: %u\n", nodes[i].devname, nodes[i].dev);
    }

    kprintf("\nmounts\n");
    for(int i = 0; i < MAX_MOUNTPOINTS; i++) {
        if(mountpoints[i].dirname)
            kprintf("dirname: %s devname: %s dev: %u sb: %x\n",
                mountpoints[i].dirname, mountpoints[i].devname,
                mountpoints[i].dev, mountpoints[i].sb);
    }

    kprintf("\n");

    open("/mnt/neris/file1");
#endif

    sti();

    //kprintf("ticks: %u\n", ticks);
    //msleep(2000);
    //kprintf("ticks: %u\n", ticks);

    //for(int i = 0; i < 4; i++)
    //    kprintf("device: %d present: %s\n", i, ide_devices[i].present ? "true" : "false");

    int pid;
    if((pid = fork()) != 0) {
        kprintf("THIS IS MY AWESOME KERNEL\n");
        kprintf("AUTHOR: MARRONY N. NERIS\n");
        kprintf("VERSION: 1.0\n\n");
        //static int count = 0;
        while(1) {
            //kprintf("main %d\n", count++);
            hlt();
        }
    } else {
        while(1) {
            //kprintf("Hi, I'm a child with pid: %d\n", getpid());
            hlt();
        }
    }

    return 0;
}

