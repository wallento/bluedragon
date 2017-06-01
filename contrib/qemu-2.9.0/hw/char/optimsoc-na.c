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

#define BUFSIZE 4096

/*
 * Number of endpoints.
 */
#define OPTIMSOC_NA_NUMEP 16

/*
 * OpTiMSoC NoC Adapter base address.
 */
#define OPTIMSOC_NA_BASE 0x00

#define OPTIMSOC_NA_REGSIZE 0x2000

/*
 * OpTiMSoC NoC Adapter endpoint registers base address.
 */
#define OPTIMSOC_NA_EP_BASE \
	(OPTIMSOC_NA_BASE + OPTIMSOC_NA_REGSIZE)

/*
 * Number of endpoints register.
 */
#define OPTIMSOC_NA_REG_NUMEP \
	(OPTIMSOC_NA_BASE)

/*
 * Send register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_SEND(i) \
	(OPTIMSOC_NA_EP_BASE + (i)*OPTIMSOC_NA_EP_REGSIZE + 0)

/*
 * Receive register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_RECV(i) \
	(OPTIMSOC_NA_EP_BASE + (i)*OPTIMSOC_NA_EP_REGSIZE + 0)

/*
 * Enable register for ith endpoint.
 */
#define OPTIMSOC_NA_REG_ENABLE(i) \
	(OPTIMSOC_NA_EP_BASE + (i)*OPTIMSOC_NA_EP_REGSIZE + 4)

#define DEBUG(str) \
	fprintf(stdout, "[QEMU] %s\n", (str));

/**
 * Device buffers.
 */
struct buffer
{
	int head;               /* Index to head data. */
	int tail;               /* Index to tail data. */
	uint32_t data[BUFSIZE]; /* Data.               */
};

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
	OPTIMSOC_NA_NUMEP,
};

struct buffer in_buffers[OPTIMSOC_NA_NUMEP];
struct buffer out_buffers[OPTIMSOC_NA_NUMEP];

/*============================================================================*
 * Buffer Operations                                                          *
 *============================================================================*/

/*
 * Initializes a buffer.
 *
 * @para buf Target buffer.
 */
static void buffer_init(struct buffer *buf)
{
	buf->head = 0;
	buf->tail = 0;
}

/*
 * Asserts if a buffer is empty.
 *
 * @param buf target buffer.
 */
static int buffer_empty(struct buffer *buf)
{
	return (buf->head == buf->tail);
}

/*
 * Asserts if a buffer is full.
 *
 * @param buf target buffer.
 */
static int buffer_full(struct buffer *buf)
{
	return ((buf->tail + 1)%BUFSIZE == buf->head);
}

/*
 * Reads a word from the target buffer.
 *
 * @addr target address.
 */
static uint32_t buffer_read(hwaddr addr)
{
	int i;
	int ep;
	uint32_t word;
	
	ep = (addr - info.numep)/OPTIMSOC_NA_REGSIZE;

	/* Empty buffer. */
	if (buffer_empty(&in_buffers[ep]))
		return (0);

	i = in_buffers[ep].head;
	word = in_buffers[ep].data[i];
	in_buffers[ep].head = (in_buffers[ep].head + 1)%BUFSIZE;

	return (word);
}

/*
 * Writes a word to the target buffer.
 *
 * @addr target address.
 */
static void buffer_write(hwaddr addr, uint32_t word)
{
	int i;
	int ep;
	
	ep = (addr - info.numep)/OPTIMSOC_NA_REGSIZE;

	/* Empty buffer. */
	if (buffer_full(&out_buffers[ep]))
	{
		DEBUG("out buffer overflow");
		return;
	}

	i = in_buffers[ep].tail;
	in_buffers[ep].data[i] = word;
	in_buffers[ep].tail = (in_buffers[ep].tail + 1)%BUFSIZE;
}

/*============================================================================*
 * Memory Mapped IO Operations                                                *
 *============================================================================*/

/*
 * Memory read callback.
 */
static uint64_t optimsoc_na_mm_read(void *opaque, hwaddr addr, unsigned size)
{
	/* Sanity check. */
	if (size != 4)
		DEBUG("bad size for read request");

	/* Read number of entry points register. */
	if (addr == OPTIMSOC_NA_REG_NUMEP)
		return (info.numep);

	DEBUG("optimsoc read");

	return (buffer_read(addr));
}

/*
 * Memory write callback.
 */
static void optimsoc_na_mm_write(void *opaque, hwaddr addr, uint64_t value, unsigned size)
{
	/* Sanity check. */
	if (size != 4)
		DEBUG("bad size for write request");

	DEBUG("optimsoc write");

	buffer_write(addr, value);
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

	/* Initialize device buffers. */
	for (int i = 0; i < OPTIMSOC_NA_NUMEP; i++)
	{
		buffer_init(&in_buffers[i]);
		buffer_init(&out_buffers[i]);
	}

	/* Initializes memory mapped IO region. */
    memory_region_init_io(&info.io, NULL, &optimsoc_na_mm_ops, NULL, "optimsoc-na",
		OPTIMSOC_NA_NUMEP*(OPTIMSOC_NA_REGSIZE + 1));

	/* Attach memory mapped IO region to the virtual address space. */
    memory_region_add_subregion(info.addrspace, info.base, &info.io);
}

