/*
 * QEMU 16550A UART emulation
 *
 * Copyright (c) 2003-2004 Fabrice Bellard
 * Copyright (c) 2008 Citrix Systems, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include "qemu/osdep.h"
#include "sysemu/char.h"
#include "qapi/error.h"
#include "qemu/timer.h"
#include "hw/irq.h"
#include "exec/address-spaces.h"
#include "qemu/error-report.h"

#define LISNOC_SIZE 8

static MemoryRegion io;

static uint64_t lisnoc_mm_read(void *opaque, hwaddr addr, unsigned size)
{
	return (0);
}

static void lisnoc_mm_write(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
	fprintf(stderr, "[QEMU] hello world from lisnoc device\n");
}

static const MemoryRegionOps lisnoc_mm_ops = {
	.read = lisnoc_mm_read,
	.write = lisnoc_mm_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

/* keep. */
void lisnoc_mm_init(MemoryRegion *address_space, hwaddr base, qemu_irq irq)
{
	/* Initializes memory mapped IO region. */
    memory_region_init_io(&io, NULL, &lisnoc_mm_ops, NULL, "lisnoc", LISNOC_SIZE);

	/* Attach memory mapped IO region to the virtual address space. */
    memory_region_add_subregion(address_space, base, &io);
}

