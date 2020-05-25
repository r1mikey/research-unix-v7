@/*
@ * This file is derived from sys/arm/arm/vfp.c from FreeBSD.
@ */
@/*-
@ * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
@ *
@ * Copyright (c) 2014 Ian Lepore <ian@freebsd.org>
@ * Copyright (c) 2012 Mark Tinguely
@ *
@ * All rights reserved.
@ *
@ * Redistribution and use in source and binary forms, with or without
@ * modification, are permitted provided that the following conditions
@ * are met:
@ * 1. Redistributions of source code must retain the above copyright
@ *    notice, this list of conditions and the following disclaimer.
@ * 2. Redistributions in binary form must reproduce the above copyright
@ *    notice, this list of conditions and the following disclaimer in the
@ *    documentation and/or other materials provided with the distribution.
@ *
@ * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
@ * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
@ * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
@ * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
@ * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
@ * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
@ * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
@ * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
@ * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
@ * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
@ * SUCH DAMAGE.
@ */
.syntax unified
.arm
.section .text
.fpu vfpv2
.fpu vfpv3

.global read_fpsid
read_fpsid:
    vmrs    r0, fpsid
    mov     pc, lr

.global read_fpexc
read_fpexc:
    vmrs    r0, fpexc
    mov     pc, lr

.global write_fpexc
write_fpexc:
    vmsr    fpexc, r0
    mov     pc, lr

.global read_mvfr0
read_mvfr0:
    vmrs    r0, mvfr0
    mov     pc, lr

.global read_mvfr1
read_mvfr1:
    vmrs    r0, mvfr1
    mov     pc, lr

.global read_fpscr
read_fpscr:
    vmrs    r0, fpscr
    mov     pc, lr

.global write_fpscr
write_fpscr:
    vmsr    fpscr, r0
    mov     pc, lr

.global read_fpinst
read_fpinst:
    vmrs    r0, fpinst
    mov     pc, lr

.global write_fpinst
write_fpinst:
    vmsr    fpinst, r0
    mov     pc, lr

.global read_fpinst2
read_fpinst2:
    vmrs    r0, fpinst2
    mov     pc, lr

.global write_fpinst2
write_fpinst2:
    vmsr    fpinst2, r0
    mov     pc, lr

.global vfp_loadregs
vfp_loadregs:
    vldmia   r0!, {d0-d15}
    cmp      r1, #0
    vldmiane r0!,  {d16-d31}
    addeq    r0, r0, #128
    mov      pc, lr

.global vfp_saveregs
vfp_saveregs:
    vstmia   r0!, {d0-d15}
    cmp      r1, #0
    vstmiane r0!, {d16-d31}
    addeq    r0, r0, #128
    mov      pc, lr
