#!/usr/bin/env python3

import os
import sys
import struct
import subprocess
import sys

N_UNDF = 0         # undefined
N_ABS  = 0o01      # absolute
N_TEXT = 0o02      # text symbol
N_DATA = 0o03      # data symbol
N_BSS  = 0o04      # bss symbol
N_TYPE = 0o037
N_REG  = 0o024     # register name
N_FN   = 0o037     # file name symbol
N_EXT  = 0o040     # external bit, or'ed in

SYMBOL_TYPE = {
    "A": N_ABS,
    "B": N_BSS,
    "b": N_BSS,
    "C": N_BSS,
    "D": N_DATA,
    "d": N_DATA,
    "G": N_DATA,
    "g": N_DATA,
    "n": N_DATA,
    "R": N_DATA,
    "r": N_DATA,
    "S": N_BSS,
    "s": N_BSS,
    "T": N_TEXT,
    "t": N_TEXT,
    "U": N_UNDF,
}

class Symbol(object):
    def __init__(self, addr, symtype, name):
        self.addr = int(addr, 16)
        self.symtype = SYMBOL_TYPE[symtype]
        self.name = name[:8]

    def __repr__(self):
        return "<<0x%08x %06o %s>>" % (self.addr, self.symtype, self.name[0:11],)

    def encode(self):
        rv = struct.pack("@8sHHI", self.name[0:10].encode('utf-8'), self.symtype, 0, self.addr)
        assert(len(rv) == 16)
        return rv

def getsyms(elf):
    syms = []
    p = subprocess.Popen(["arm-none-eabi-nm", elf], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, _ = p.communicate()
    if p.returncode != 0:
        return []
    for line in out.decode('utf-8').strip().split("\n"):
        chunks = line.split(" ")
        if chunks[1] not in ("a",):
            syms.append(Symbol(*chunks))
    return syms

if not len(sys.argv) in (5, 6):
    print("Usage: mkinit-aout.py [-s] STEM TEXT-SIZE DATA-SIZE BSS-SIZE")
    sys.exit(1)

if len(sys.argv) == 5:
    do_symtab = False
    BINFILE = "{}.bin".format(sys.argv[1])
else:
    if sys.argv[1] == "-s":
        do_symtab = True
    else:
        print("Usage: mkinit-aout.py [-s] STEM TEXT-SIZE DATA-SIZE BSS-SIZE")
        sys.exit(1)
    BINFILE = "{}.bin".format(sys.argv[2])

if not os.path.exists(BINFILE):
    print("Usage: mkinit-aout.py [-s] STEM TEXT-SIZE DATA-SIZE BSS-SIZE")
    sys.exit(1)

try:
    if len(sys.argv) == 5:
        txtsz = int(sys.argv[2])
        datsz = int(sys.argv[3])
        bsssz = int(sys.argv[4])
    else:
        txtsz = int(sys.argv[3])
        datsz = int(sys.argv[4])
        bsssz = int(sys.argv[5])
except ValueError:
    print("Usage: mkinit-aout.py [-s] STEM TEXT-SIZE DATA-SIZE BSS-SIZE")
    sys.exit(1)

txtsz = ((txtsz + 4095) & ~(4095))

if len(sys.argv) == 5:
    AOUTFILE = "{}.aout".format(sys.argv[1])
    ELFFILE = "{}.elf".format(sys.argv[1])
else:
    AOUTFILE = "{}.aout".format(sys.argv[2])
    ELFFILE = "{}.elf".format(sys.argv[2])

magic = 0o407
entrypoint = 0
symssz = 0
symtab = b''

if do_symtab:
    syms = getsyms(ELFFILE)
    for sym in syms:
        symtab += sym.encode()
    symssz = len(symtab)

header = struct.pack("@IIIIIIII",
    magic,
    txtsz,
    datsz,
    bsssz,
    symssz,
    entrypoint,
    0,
    1,
)

with open(BINFILE, 'rb') as infile:
    with open(AOUTFILE, 'wb+') as outfile:
        outfile.write(header)
        outfile.write(infile.read())
        if symssz:
            outfile.write(symtab)
