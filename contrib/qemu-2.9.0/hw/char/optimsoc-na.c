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

#define DEBUG(s) \
	printf("[QEMU] %s\n", s)

/**
 * @brief NoC Adapter buffer size (in bytes).
 */
#define BUFSIZE 4096

/*
 * Number of endpoints.
 */
#define OPTIMSOC_NUMEP 16

/**
 * @brief NoC Adapter base address.
 */
#define OPTIMSOC_BASE 0x0000

/**
 * @brief NoC Adapter offsets.
 */
#define OPTIMSOC_OFF 0x2000

/**
 * @brief NoC Adapter register mask.
 */
#define OPTIMSOC_REG_MASK 0xf

/**
 * @brief Register base addresses.
 */
/**@{*/
#define OPTIMSOC_REG_BASE_NUMEP  (OPTIMSOC_BASE)                /**< Number of endpoints. */
#define OPTIMSOC_REG_BASE_SEND   (OPTIMSOC_BASE + OPTIMSOC_OFF) /**< Send.                */
#define OPTIMSOC_REG_BASE_RECV   (OPTIMSOC_BASE + OPTIMSOC_OFF) /**< Receive.             */
#define OPTIMSOC_REG_BASE_ENABLE (OPTIMSOC_BASE + OPTIMSOC_OFF) /**< Enable.              */
/**@}*/

/**
 * @brief Register offsets.
 */
/**@{*/
#define OPTIMSOC_REG_OFF_NUMEP  0x0 /**< Number of endpoints.  */
#define OPTIMSOC_REG_OFF_SEND   0x0 /**< Send.                 */
#define OPTIMSOC_REG_OFF_RECV   0x0 /**< Receive.              */
#define OPTIMSOC_REG_OFF_ENABLE 0x4 /**< Enable.               */
/**@}*/

/**
 * @brief Registers.
 */
#define OPTIMSOC_REG_NUMEP        (OPTIMSOC_REG_BASE_NUMEP + OPTIMSOC_REG_OFF_NUMEP)                                  /**< Number of endpoints.  */
#define OPTIMSOC_REG_SEND(addr)   (((addr) - (OPTIMSOC_REG_BASE_SEND + OPTIMSOC_REG_BASE_SEND))/OPTIMSOC_OFF)     /**< Send.                 */
#define OPTIMSOC_REG_RECV(addr)   (((addr) - (OPTIMSOC_REG_BASE_RECV + OPTIMSOC_REG_BASE_RECV))/OPTIMSOC_OFF)     /**< Receive.              */
#define OPTIMSOC_REG_ENABLE(addr) (((addr) - (OPTIMSOC_REG_BASE_ENABLE + OPTIMSOC_REG_BASE_ENABLE))/OPTIMSOC_OFF) /**< Enable.               */
/**@}*/

/*
 * NoC Adapter information.
 */
static struct
{
	MemoryRegion *addrspace;     /**< Residing address space.     */
	MemoryRegion io;             /**< Memory mapped IO region.    */
	hwaddr base;                 /**< Base address.               */
	qemu_irq irq;                /*< IRQ.                         */
	int enabled[OPTIMSOC_NUMEP]; /**< Enabled registered address. */
} info;

/*============================================================================*
 * Buffer Operations                                                          *
 *============================================================================*/

/**
 * Device buffers.
 */
struct buffer
{
	int head;               /* Index to head data. */
	int tail;               /* Index to tail data. */
	uint32_t data[BUFSIZE]; /* Data.               */
};

struct buffer buffers[OPTIMSOC_NUMEP];

/*
 * Initializes a buffer.
 */
static void buffer_init(void)
{
	for (int i = 0; i < OPTIMSOC_NUMEP; i++)
		buffers[i].head = buffers[i].tail = 0;
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

	/* Bad register address. */
	if ((addr & OPTIMSOC_REG_MASK) != OPTIMSOC_REG_OFF_RECV)
		DEBUG("bad register address");
	
	ep = OPTIMSOC_REG_RECV(addr);

	/* Empty buffer. */
	if (buffer_empty(&buffers[ep]))
		return (0);

	i = buffers[ep].head;
	word = buffers[ep].data[i];
	buffers[ep].head = (buffers[ep].head + 1)%BUFSIZE;

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

	/* Enable operation. */
	if ((addr & OPTIMSOC_REG_MASK) == OPTIMSOC_REG_OFF_ENABLE)
	{
		ep = OPTIMSOC_REG_ENABLE(addr);
		info.enabled[ep] = 1;

		return;
	}
	
	ep = OPTIMSOC_REG_SEND(addr);

	/* Empty buffer. */
	if (buffer_full(&buffers[ep]))
	{
		DEBUG("out buffer overflow");
		return;
	}

	i = buffers[ep].tail;
	buffers[ep].data[i] = word;
	buffers[ep].tail = (buffers[ep].tail + 1)%BUFSIZE;

	qemu_irq_raise(info.irq);
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
	if (addr == OPTIMSOC_REG_NUMEP)
		return (OPTIMSOC_NUMEP);

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
	memset(info.enabled, 0, OPTIMSOC_NUMEP*sizeof(int));

	/* Initialize device buffers. */
	buffer_init();

	/* Initializes memory mapped IO region. */
    memory_region_init_io(&info.io, NULL, &optimsoc_na_mm_ops, NULL, "optimsoc-na",
		(OPTIMSOC_NUMEP + 1)*OPTIMSOC_OFF);

	/* Attach memory mapped IO region to the virtual address space. */
    memory_region_add_subregion(info.addrspace, info.base, &info.io);
}

