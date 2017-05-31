/*
 * QEMU OpTiMSoC NoC Adapter Emulation (Loopback Mode)
 *
 * Copyright (c) 2017 Pedro H. Penna
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

#include <assert.h>
#include <stdio.h>

#include "qemu/osdep.h"
#include "sysemu/char.h"
#include "qapi/error.h"
#include "qemu/timer.h"
#include "hw/irq.h"
#include "exec/address-spaces.h"
#include "qemu/error-report.h"
#include "hw/char/optimsoc-na.h"

/*
 * Number of endpoints.
 */
#define OPTIMSOC_NUMEP 256

#define OPTIMSOC_NA_REGSIZE 0x2000

#define OPTIMSOC_NA_EP_BASE(base) \
	((base) + OPTIMSOC_NA_REG_SIZE)

/*
 * Number of endpoints register.
 */
#define OPTIMSOC_NA_REG_NUMEP(base) \
	(base)

/*
 * Send register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_SEND(base,i) \
	(OPTIMSOC_NA_EP_BASE(base) + (i)*OPTIMSOC_NA_EP_REGSIZE + 0)

/*
 * Receive register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_RECV(base,i) \
	(OPTIMSOC_NA_EP_BASE(base) + (i)*OPTIMSOC_NA_EP_REGSIZE + 0)

/*
 * Enable register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_ENABLE(base,i) \
	(OPTIMSOC_NA_EP_BASE(base) + (i)*OPTIMSOC_NA_EP_REGSIZE + 4)

/*
 * NoC Adapter information.
 */
static struct
{
	/* Device information. */
	int numep;       /* Number of end points.    */

	/* Platform information. */
	MemoryRegion *addrspace; /* Residing address space.  */
	MemoryRegion io;         /* Memory mapped IO region. */
	hwaddr base;             /* Base address.            */
	qemu_irq irq;            /* IRQ.                     */
} info = {
	OPTIMSOC_NUMEP,
};

/*
 * Memory read callback.
 */
static uint64_t optimsoc_na_mm_read(void *opaque, hwaddr addr, unsigned size)
{
	fprintf(stderr, "[QEMU] get data from me\n");
	return (0);
}

/*
 * Memory write callback.
 */
static void optimsoc_na_mm_write(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
	fprintf(stderr, "[QEMU] feed me with data\n");
}

/*
 * Operations on memory mapped IO region.
 */
static const MemoryRegionOps optimsoc_na_mm_ops = {
	.read = optimsoc_na_mm_read,
	.write = optimsoc_na_mm_write,
	.endianness = DEVICE_NATIVE_ENDIAN,
};

/*
 * Initializes OpTiMSoC network memory mapped IO region.
 */
void optimsoc_na_mm_init(MemoryRegion *addrspace, hwaddr base, qemu_irq irq)
{
	/* Sanity check.  */
	assert(addrspace != NULL);

	/* Store device parameters. */
	info.addrspace = addrspace;
	info.base = base;
	info.irq = irq;

	/* Initializes memory mapped IO region. */
    memory_region_init_io(&info.io, NULL, &optimsoc_na_mm_ops, NULL, "optimsoc-na",
		OPTIMSOC_NUMEP*(OPTIMSOC_NA_REGSIZE + 1));

	/* Attach memory mapped IO region to the virtual address space. */
    memory_region_add_subregion(info.addrspace, info.base, &info.io);
}

