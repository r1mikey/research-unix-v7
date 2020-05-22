#!/usr/bin/env python3

from collections import deque
import os
import time
import sys
import struct

"""
struct  filsys {
        u16 s_isize;            /* size in blocks of i-list */
        v7_daddr_t s_fsize;        /* size in blocks of entire volume */
        i16     s_nfree;        /* number of addresses in s_free */
        v7_daddr_t s_free[NICFREE];/* free block list */
        i16     s_ninode;       /* number of i-nodes in s_inode */
        v7_ino_t   s_inode[NICINOD];/* free i-node list */
        char    s_flock;        /* lock during free list manipulation */
        char    s_ilock;        /* lock during i-list manipulation */
        char    s_fmod;         /* super block modified flag */
        char    s_ronly;        /* mounted read-only flag */
        v7_time_t  s_time;         /* last super block update */
        /* remainder not maintained by this version of the system */
        v7_daddr_t s_tfree;        /* total free blocks*/
        v7_ino_t   s_tinode;       /* total free inodes */
        i16     s_m;            /* interleave factor */
        i16     s_n;            /* " " */
        char    s_fname[6];     /* file system name */
        char    s_fpack[6];     /* file system pack name */
};

struct dinode
{
        u16     di_mode;           /* 0   2   2   mode and type of file */
        i16     di_nlink;          /* 2   2   4   number of links to file */
        i16     di_uid;            /* 4   2   6   owner's user id */
        i16     di_gid;            /* 6   2   8   owner's group id */
        v7_off_t   di_size;        /* 8   4   12  number of bytes in file */
        char    di_addr[40];       /* 12  40  52  disk block addresses (13 * 3 byte addresses) - last three are indirect, double-indirect, triple indirect */
        v7_time_t  di_atime;       /* 52  4   56  time last accessed */
        v7_time_t  di_mtime;       /* 56  4   60  time last modified */
        v7_time_t  di_ctime;       /* 60  4   64  time created */
};
#define INOPB   8       /* 8 inodes per block */
#define NADDR   13      /* number of addressed blocks in a inode: 0..NADDR-4 are direct, -3 is indirect, -2 is double indirect, -1 is triple indorect */

#define MAXFN   500

int     f_n     = MAXFN;
int     f_m     = 3;

"""

CONSTRUCTION_TS = int(time.time())

BSIZE = 512               # size of secondary block (bytes)
NICINOD = 100             # number of superblock inodes
NICFREE = 50              # number of superblock free blocks
MAX_FN = 500
DEFAULT_FN = MAX_FN
DEFAULT_FM = 3
NIPB = 8
NADDR = 13
NUM_DIRECT_ADDR = NADDR - 3
NINDIR = int(BSIZE / 4)
BYTES_PER_DIRENT = 16

IFMT  = 0o0170000  # type of file
IFCHR = 0o0020000  # character special
IFDIR = 0o0040000  # directory
IFBLK = 0o0060000  # block special
IFREG = 0o0100000  # regular
ISUID = 0o04000    # set user id on execution
ISGID = 0o02000    # set group id on execution



class FilesystemSpec(object):
    SUPERBLOCK_BLOCK_NUMBER = 1

    def __init__(self, bootblock, total_blocks, total_inodes):
        self._bootblock = bootblock
        self._total_blocks = total_blocks
        self._total_inodes = total_inodes
        self._root = None
        self._allocated_inode = 0
        self._allocated_block = 0
        self._available_inodes = self._total_blocks
        self._available_blocks = self._total_inodes

        self._fs_s_isize = int((self._total_inodes / NIPB) + 3)  # size in blocks of i-list
        self._fs_s_fsize = self._total_blocks                    # size in blocks of entire volume
        self._fs_s_nfree = 0                                     # number of addresses in s_free
        self._fs_s_free = None                                   # free block list
        self._fs_s_ninode = 0                                    # number of i-nodes in s_inode
        self._fs_s_inode = None                                  # free i-node list
        self._fs_s_flock = '\0'                                  # lock during free list manipulation
        self._fs_s_ilock = '\0'                                  # lock during i-list manipulation
        self._fs_s_fmod = '\0'                                   # super block modified flag
        self._fs_s_ronly = '\0'                                  # mounted read-only flag
        self._fs_s_time = 0                                      # last super block update
        # remainder not maintained by this version of the system
        self._fs_s_tfree = 0                                     # total free blocks
        self._fs_s_tinode = 0                                    # total free inodes
        self._fs_s_m = DEFAULT_FM                                # interleave factor
        self._fs_s_n = DEFAULT_FN                                # ...
        self._fs_s_fname = ""                                    # file system name
        self._fs_s_fpack = ""                                    # file system pack name

        self.free_blocks = deque()
        self.free_inodes = [x for x in range(1, self._total_inodes + 1)]
        self.free_inodes.reverse()

        print("m/n = {} {}".format(self._fs_s_m, self._fs_s_n))
        if self._fs_s_isize >= self._fs_s_fsize:
            raise RuntimeError("{}/{}: bad ratio".format(self._fs_s_fsize, self._fs_s_isize - 2))

        self._bad_block_table_inode = IndexNode(self, IFREG, 0, 0, 0)
        self._bad_block_table_inode.set_source_file(None)
        assert(self._bad_block_table_inode._inum == 1)

    def set_root(self, root):
        assert(root._inode._inum == 2)
        self._root = root

    # the free list ends up rooted at inode 1 as an unreferenced regular file (from what I can see)
    # inode 2 is the fs root
    # everything else is allocated as needed
    # blocks start at 1 (superblock), followed by the free list head and the inodes, then files/indirects and free list blocks
    def build_freelist(self):
        flg = [0 for _ in range(0, self._fs_s_n)]
        adr = [0 for _ in range(0, self._fs_s_n)]

        i = 0
        for j in range(0, self._fs_s_n):
            while flg[i]:
                i = int((i + 1) % self._fs_s_n)
            adr[j] = i + 1
            flg[i] = flg[i] + 1
            i = int((i + self._fs_s_m) % self._fs_s_n)

        d = self._fs_s_fsize - 1
        if d % self._fs_s_n == 0:
            d += 1
        while d % self._fs_s_n:
            d += 1

        while d:
            for i in range(0, self._fs_s_n):
                f = d - adr[i]
                if f < self._fs_s_fsize and f >= self._fs_s_isize:
                    self.free_blocks.append(f)
            d -= self._fs_s_n

    def claim_inode(self):
        return self.free_inodes.pop()

    def claim_block(self):
        return self.free_blocks.pop()

    def _store_freelist(self, fh):
        pad = '\0' * (BSIZE - (NICFREE * 4))
        sfmt = "<{}I{}s".format(NICFREE, (BSIZE - (NICFREE * 4)))

        blocks = [bno for bno in self.free_blocks]
        blocks.reverse()
        remaining = len(blocks)
        pos = 0
        while remaining:
            n = min(remaining, NICFREE)
            remaining -= n
            chunk = blocks[pos:pos + n]
            pos += n
            if n != NICFREE:
                chunk += [0 for _ in range(0, (NICFREE - n))]
            assert(len(chunk) == NICFREE)
            if not self._fs_s_free:
                self._fs_s_free = chunk[:]
                self._fs_s_nfree = n
                bno = self._fs_s_free[0]
                continue
            assert((len(chunk) * 4) + len(pad) == BSIZE)
            args = chunk + [pad.encode("utf-8")]
            data = struct.pack(sfmt, *args)
            assert(len(data) == BSIZE)
            fh.seek(bno * BSIZE)
            written = fh.write(data)
            if written != BSIZE:
                raise RuntimeError("Failed to write free-space block - only wrote {} of {} bytes".format(written, BSIZE))
            bno = chunk[0]

    def _store_self(self, fh):
        self._fs_s_tfree = len(self.free_blocks)
        self._fs_s_tinode = len(self.free_inodes)
        self._fs_s_time = int(time.time())
        self._fs_s_inode = self.free_inodes[0:NICINOD]
        self._fs_s_ninode = len(self._fs_s_inode)
        if len(self._fs_s_inode) != NICINOD:
            self._fs_s_inode += [0 for _ in range(0, (NICINOD - len(self._fs_s_inode)))]
        assert(len(self._fs_s_inode) == NICINOD)
        assert(len(self._fs_s_free) == NICFREE)

        sfmt = "@Hih{}ih{}HcccciiHhh6s6s".format(NICFREE, NICINOD)
        args = [
            self._fs_s_isize,
            self._fs_s_fsize,
            self._fs_s_nfree,
        ] + self._fs_s_free + [
            self._fs_s_ninode,
        ] + self._fs_s_inode + [
            self._fs_s_flock.encode("utf-8"),
            self._fs_s_ilock.encode("utf-8"),
            self._fs_s_fmod.encode("utf-8"),
            self._fs_s_ronly.encode("utf-8"),
            self._fs_s_time,
            self._fs_s_tfree,
            self._fs_s_tinode,
            self._fs_s_m,
            self._fs_s_n,
            self._fs_s_fname.encode("utf-8"),
            self._fs_s_fpack.encode("utf-8"),
        ]
        data = struct.pack(sfmt, *args)
        EXPECTED_DATA_LENGTH = 446  # 440 if unpadded
        assert(len(data) == EXPECTED_DATA_LENGTH)
        pad = b'\0' * (BSIZE - EXPECTED_DATA_LENGTH)
        data += pad

        fh.seek(FilesystemSpec.SUPERBLOCK_BLOCK_NUMBER * BSIZE)
        written = fh.write(data)
        if written != BSIZE:
            raise RuntimeError("Failed to write superblock - only wrote {} of {} bytes".format(written, BSIZE))

    def store(self, fh):
        self._bad_block_table_inode.store(self, fh)
        self._root.store(self, fh)
        self._store_freelist(fh)
        self._store_self(fh)


class DirectoryEntry(object):
    DIRECT_SIZE = 16

    def __init__(self, inum, name):
        self._inum = inum
        self._name = name

    def __lt__(self, other):
        return self._name < other._name

    def as_direct(self):
        d = struct.pack("@H14s", self._inum, self._name.encode("utf-8"))
        assert(len(d) == DirectoryEntry.DIRECT_SIZE)
        return d

"""
/*
 * Inode structure as it appears on
 * a disk block.
 */
struct dinode
{
        u16     di_mode;           /* 0   2   2   mode and type of file */
        i16     di_nlink;          /* 2   2   4   number of links to file */
        i16     di_uid;            /* 4   2   6   owner's user id */
        i16     di_gid;            /* 6   2   8   owner's group id */
        v7_off_t   di_size;        /* 8   4   12  number of bytes in file */
        char    di_addr[40];       /* 12  40  52  disk block addresses (13 * 3 byte addresses) */
        v7_time_t  di_atime;       /* 52  4   56  time last accessed */
        v7_time_t  di_mtime;       /* 56  4   60  time last modified */
        v7_time_t  di_ctime;       /* 60  4   64  time created */
};
#define INOPB   8       /* 8 inodes per block */

#define IFMT    0170000         /* type of file */
#define         IFCHR   0020000 /* character special */
#define         IFDIR   0040000 /* directory */
#define         IFBLK   0060000 /* block special */
#define         IFREG   0100000 /* regular */
#define ISUID   04000           /* set user id on execution */
#define ISGID   02000           /* set group id on execution */

dev/mx1.c:                      ip->i_mode = 0666+IFCHR;

"""
#define itod(x) (v7_daddr_t)((((unsigned)x+15)>>3))
#define itoo(x) (int)((x+15)&07)

def itod(x):
    return (x + 15) >> 3

def itoo(x):
    return (x + 15) & 0o07
#
# Pass  1: discover all nodes from the config
# Pass  2: allocate index nodes
# Pass  3: set children (now that we have inodes - sets up dirents as needed - maybe create dirents and collect them?)
# Pass  4: set up storage (now that we have files and directories)
# Pass  5: allocate storage for the free block list
# Pass  6: store data blocks to disk
# Pass  7: store the free block list
# Pass  8: store index nodes, clear unused inode blocks
# Pass  9: store the superblock
# Pass 10: store the bootblock
#
# Index node number 1 contains the free list, index node number 2 contains the root inode.
#

class BlockTable(object):
    NDIRECT = 10     # number of direct data blocks
    SINDIR_IDX = 10  # direct index of the single-indirect directory block number
    DINDIR_IDX = 11  # direct index of the double-indirect directory block number
    TINDIR_IDX = 12  # direct index of the triple-indirect directory block number

    def __init__(self):
        self._leaf_blocks = []
        self._directs = [0 for _ in range(0, 13)]
        self._indirects = []  # tuple(bno, [blocks])

    @property
    def i_addr(self):
        return self._directs

    @property
    def all_blocks_consumed(self):
        return len(self._leaf_blocks) == 0

    def _alloc_direct(self, allocator, remaining):
        n = min(remaining, BlockTable.NDIRECT)
        for x in range(0, n):
            bno = allocator.claim_block()
            self._directs[x] = bno
            self._leaf_blocks.append(bno)
        return remaining - n

    def _alloc_single_indirect(self, allocator, remaining):
        if not remaining:
            return 0, 0
        blockdir = [0 for _ in range(0, NINDIR)]
        ibno = allocator.claim_block()
        n = min(remaining, NINDIR)
        for x in range(0, n):
            bno = allocator.claim_block()
            blockdir[x] = bno
            self._leaf_blocks.append(bno)
        self._indirects.append((ibno, blockdir,))
        return ibno, (remaining - n)

    def _alloc_double_indirect(self, allocator, remaining):
        if not remaining:
            return 0, 0
        rem = remaining
        blockdir = [0 for _ in range(0, NINDIR)]
        ibno = allocator.claim_block()
        for d in range(0, NINDIR):
            bno, rem = self._alloc_single_indirect(allocator, rem)
            blockdir[d] = bno
            if not rem:
                break
        self._indirects.append((ibno, blockdir,))
        return ibno, rem

    def _alloc_triple_indirect(self, allocator, remaining):
        if not remaining:
            return 0, 0
        rem = remaining
        blockdir = [0 for _ in range(0, NINDIR)]
        ibno = allocator.claim_block()
        for d in range(0, NINDIR):
            bno, rem = self._alloc_double_indirect(allocator, rem)
            blockdir[d] = bno
            if not rem:
                break
        self._indirects.append((ibno, blockdir,))
        return ibno, rem

    def setup_for_size(self, allocator, size):
        nblk = int(((size + (BSIZE - 1)) & ~(BSIZE - 1)) / BSIZE)
        remaining = self._alloc_direct(allocator, nblk)
        if remaining:
            bno, remaining = self._alloc_single_indirect(allocator, remaining)
            self._directs[BlockTable.SINDIR_IDX] = bno
        if remaining:
            bno, remaining = self._alloc_double_indirect(allocator, remaining)
            self._directs[BlockTable.DINDIR_IDX] = bno
        if remaining:
            bno, remaining = self._alloc_triple_indirect(allocator, remaining)
            self._directs[BlockTable.TINDIR_IDX] = bno
        if remaining:
            raise RuntimeError(
                "File too large.  Failed to place {} of {} blocks.".format(
                    remaining, nblk)
            )
        self._leaf_blocks.reverse()
 
    def store_next_block(self, fh, data):
        assert(len(data) <= BSIZE)
        if len(data) != BSIZE:
            data += (b'\0' * (BSIZE - len(data)))
        assert(len(data) == BSIZE)
        bno = self._leaf_blocks.pop()
        fh.seek(bno * BSIZE)
        n = fh.write(data)
        if n != BSIZE:
            raise RuntimeError("Failed to write all data in block {}: only wrote {} of {} bytes".format(bno, n, BSIZE))

    def store_indirects(self, fh):
        for bno, dirtab in self._indirects:
            assert(len(dirtab) == NINDIR)
            data = struct.pack("@{}I".format(NINDIR), *dirtab)
            assert(len(data) == NINDIR * 4)
            if len(data) < BSIZE:
                data += (b'\0' * (BSIZE - len(data)))
            assert(len(data) == BSIZE)
            fh.seek(bno * BSIZE)
            n = fh.write(data)
            if n != BSIZE:
                raise RuntimeError("Failed to write all data in indirect table block {}: only wrote {} of {} bytes".format(bno, n, BSIZE))


TYPETEXT = {
    IFCHR: "CHR",
    IFDIR: "DIR",
    IFBLK: "BLK",
    IFREG: "REG",
}

class IndexNode(object):
    DINODE_SIZE = 64

    def __init__(self, ialloc, fmt, mode, uid, gid):
        self._inum = ialloc.claim_inode() # claim an i-number for this node
        self._dev_major = None            # device major
        self._dev_minor = None            # device minor
        self._reg_source = None           # string, up to 14 characters
        self._dirents = None              # list of tuples of (d_ino, d_name) - do we actually need this?
        self._blocks_used = 0             # number of logical blocks used (does not include tables)

        self._di_mode = fmt | mode        # mode and type of file   - 2 bytes
        self._di_nlink = 1                # number of links to file - 2 bytes
        self._di_uid = uid                # owner's user id         - 2 bytes
        self._di_gid = gid                # owner's group id        - 2 bytes
        self._di_size = 0                 # number of bytes in file - 4 bytes
        # self._di_addr                   # disk block addresses    - 40 bytes
        self._di_atime = CONSTRUCTION_TS  # time last accessed      - 4 bytes
        self._di_mtime = CONSTRUCTION_TS  # time last modified      - 4 bytes
        self._di_ctime = CONSTRUCTION_TS  # time created            - 4 bytes

    def is_type(self, t):
        return (self._di_mode & IFMT) == t

    def set_device_info(self, major, minor):
        if (self._di_mode & IFMT) not in (IFCHR, IFBLK,):
            raise RuntimeError("Attempt to set device major/minor on something that is not a device")
        self._dev_major = major
        self._dev_minor = minor

    def set_source_file(self, path):
        if (self._di_mode & IFMT) != IFREG:
            raise RuntimeError("Attempt to set source on something that is not a regular file")
        if path:
            st = os.stat(path)
            self._di_size = st.st_size
            self._di_atime = int(st.st_atime)
            self._di_mtime = int(st.st_mtime)
            self._di_ctime = int(st.st_ctime)
            self._reg_source = path
        else:
            self._di_size = 0
            self._reg_source = None

    def _pack_dinode(self, i_addr):
        assert(len(i_addr) == 13)
        return struct.pack("@HhhhIBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBiii",
            self._di_mode,
            self._di_nlink,
            self._di_uid,
            self._di_gid,
            self._di_size,
            i_addr[0] & 0xff, (i_addr[0] >> 8) & 0xff, (i_addr[0] >> 16) & 0xff,
            i_addr[1] & 0xff, (i_addr[1] >> 8) & 0xff, (i_addr[1] >> 16) & 0xff,
            i_addr[2] & 0xff, (i_addr[2] >> 8) & 0xff, (i_addr[2] >> 16) & 0xff,
            i_addr[3] & 0xff, (i_addr[3] >> 8) & 0xff, (i_addr[3] >> 16) & 0xff,
            i_addr[4] & 0xff, (i_addr[4] >> 8) & 0xff, (i_addr[4] >> 16) & 0xff,
            i_addr[5] & 0xff, (i_addr[5] >> 8) & 0xff, (i_addr[5] >> 16) & 0xff,
            i_addr[6] & 0xff, (i_addr[6] >> 8) & 0xff, (i_addr[6] >> 16) & 0xff,
            i_addr[7] & 0xff, (i_addr[7] >> 8) & 0xff, (i_addr[7] >> 16) & 0xff,
            i_addr[8] & 0xff, (i_addr[8] >> 8) & 0xff, (i_addr[8] >> 16) & 0xff,
            i_addr[9] & 0xff, (i_addr[9] >> 8) & 0xff, (i_addr[9] >> 16) & 0xff,
            i_addr[10] & 0xff, (i_addr[10] >> 8) & 0xff, (i_addr[10] >> 16) & 0xff,
            i_addr[11] & 0xff, (i_addr[11] >> 8) & 0xff, (i_addr[11] >> 16) & 0xff,
            i_addr[12] & 0xff, (i_addr[12] >> 8) & 0xff, (i_addr[12] >> 16) & 0xff,
            0,
            self._di_atime,
            self._di_mtime,
            self._di_ctime,
        )

    def _store_self(self, fh, i_addr):
        dinode = self._pack_dinode(i_addr)
        assert(len(dinode) == IndexNode.DINODE_SIZE)
        fh.seek((itod(self._inum) * BSIZE) + (itoo(self._inum) * IndexNode.DINODE_SIZE))
        if fh.write(dinode) != IndexNode.DINODE_SIZE:
            raise RuntimeError("Failed to write special file entry")

    def _store_spc(self, fh):
        assert(self._dev_major != None)
        assert(self._dev_minor != None)
        i_addr = [0 for _ in range(0, 13)]
        i_addr[0] = (self._dev_major << 8) | self._dev_minor
        self._store_self(fh, i_addr)

    def _store_dir(self, allocator, fh, directs):
        data = b""
        for d in sorted(directs):
            data += d.as_direct()
        self._di_size = len(data)
        tab = BlockTable()
        tab.setup_for_size(allocator, self._di_size)
        while data:
            tab.store_next_block(fh, data[:BSIZE])
            data = data[BSIZE:]
        if not tab.all_blocks_consumed:
            raise RuntimeError("Not all blocks consumed for directory")
        tab.store_indirects(fh)
        self._store_self(fh, tab.i_addr)

    def _store_reg(self, allocator, fh):
        tab = BlockTable()
        tab.setup_for_size(allocator, self._di_size)
        if self._di_size:
            remaining = self._di_size
            with open(self._reg_source, "rb") as sfh:
                while remaining:
                    n = min(remaining, BSIZE)
                    remaining -= n
                    data = sfh.read(n)
                    if len(data) != n:
                        raise RuntimeError("Short read from source file {}".format(self._reg_source))
                    tab.store_next_block(fh, data)
        if not tab.all_blocks_consumed:
            raise RuntimeError("Not all blocks consumed for regular file")
        tab.store_indirects(fh)
        self._store_self(fh, tab.i_addr)

    def store(self, allocator, fh, extra=None):
        if (self._di_mode & IFMT) in (IFBLK, IFCHR,):
            self._store_spc(fh)
        elif (self._di_mode & IFMT) == IFDIR:
            self._store_dir(allocator, fh, extra)
        elif (self._di_mode & IFMT) == IFREG:
            self._store_reg(allocator, fh)


class FilesystemRecord(object):
    def __init__(self, ialloc, parent, name, mode, uid, gid, src_maj=None, minor=None):
        self._parent = parent
        self._omode = mode
        self.directs = []
        self.children = []
        self._is_open = True

        if len(mode) != 6:
            raise RuntimeError("Bad mode length (must be 6, was {}): \"{}\"".format(len(mode), mode))

        if mode[1] != '-' or mode[2] != '-':
            raise RuntimeError("Unhandled mode: {}".format(mode))

        perms = int(mode[3:], 8)
        self._mode = perms
        self._uid = int(uid)
        self._gid = int(gid)
        self._name = name

        if mode[0] == '-':
            self._inode = IndexNode(ialloc, IFREG, perms, self._uid, self._gid)
        elif mode[0] == 'd':
            self._inode = IndexNode(ialloc, IFDIR, perms, self._uid, self._gid)
        elif mode[0] == 'c':
            self._inode = IndexNode(ialloc, IFCHR, perms, self._uid, self._gid)
        elif mode[0] == 'b':
            self._inode = IndexNode(ialloc, IFBLK, perms, self._uid, self._gid)
        else:
            raise RuntimeError("Unknown format: {}".format(mode[0]))

        if (self._inode._di_mode & IFMT) in (IFCHR, IFBLK,):
            self._major = int(src_maj)
            self._minor = int(minor)
            self._inode.set_device_info(self._major, self._minor)
        elif (self._inode._di_mode & IFMT) == IFREG:
            if src_maj is None:
                raise RuntimeError("File entries must have a source")
            self._source = src_maj
            self._inode.set_source_file(self._source)
        elif (self._inode._di_mode & IFMT) == IFDIR:
            par = self._parent
            if par is None:
                par = self
                par._inode._di_nlink -= 1
            self.directs.append(DirectoryEntry(self._inode._inum, "."))
            self._inode._di_nlink += 1
            self.directs.append(DirectoryEntry(par._inode._inum, ".."))
            par._inode._di_nlink += 1

    def close(self):
        if not self._inode.is_type(IFDIR):
            return
        self._is_open = False

    def is_dir(self):
        return self._inode.is_type(IFDIR)

    def add_child(self, item):
        if not self.is_dir():
            raise RuntimeError("Attempt to add a child to a non-directory")
        if not self._is_open:
            raise RuntimeError("Failing an attempt to add a child ({}) to a closed directory ({})".format(item._name, self._name))
        self.children.append(item)
        self.directs.append(DirectoryEntry(item._inode._inum, item._name))
        # item._inode._di_nlink already starts at 1

    def __lt__(self, other):
        return self._name < other._name

    def store(self, allocator, fh):
        if (self._inode._di_mode & IFMT) in (IFCHR, IFBLK,):
            self._inode.store(allocator, fh)
        elif (self._inode._di_mode & IFMT) == IFDIR:
            self._inode.store(allocator, fh, self.directs)
        elif (self._inode._di_mode & IFMT) == IFREG:
            self._inode.store(allocator, fh)
        else:
            raise RuntimeError("Don't know how to store nodes of type {%o}".format(self._inode._di_mode & IFMT))
        for c in self.children:
            c.store(allocator, fh)


def _parse_entry(fsspec, parent, line):
    parts = [x.strip() for x in line.split()]
    return FilesystemRecord(fsspec, parent, *parts)


def parse_proto(path):
    fsspec = None
    bootblock = None
    total_blocks = None
    total_inodes = None
    root = None
    dirstack = []
    with open(path, "r") as proto:
        for line in proto:
            line = line.strip()
            if bootblock is None:
                bootblock = line
                print("set bootblock to: {}".format(bootblock))
                continue
            if total_blocks is None and total_inodes is None:
                total_blocks, total_inodes = [int(x) for x in line.split(' ')]
                print("set totals to {} blocks, {} inodes".format(total_blocks, total_inodes))
                continue
            if bootblock is None or total_blocks is None or total_inodes is None:
                raise RuntimeError("bad file format")
            if fsspec is None:
                fsspec = FilesystemSpec(bootblock, total_blocks, total_inodes)
            if root is None:
                root = _parse_entry(fsspec, None, "/ " + line)
                dirstack.append(root)
                fsspec.set_root(root)
            else:
                if line == "$":
                    item = dirstack.pop()
                    item.close()
                    if not dirstack:
                        break
                    continue
                item = _parse_entry(fsspec, dirstack[-1], line)
                dirstack[-1].add_child(item)
                if item.is_dir():
                    dirstack.append(item)
    return fsspec


def main():
    if len(sys.argv) > 1:
        fs = parse_proto(sys.argv[1])
    else:
        fs = parse_proto("v7-rpi.proto")
    fs.build_freelist()
    if len(sys.argv) > 2:
        fn = sys.argv[2]
    else:
        fn = "abc.fs"
    with open(fn, "wb+") as fh:
        fs.store(fh)


if __name__ == '__main__':
    main()
