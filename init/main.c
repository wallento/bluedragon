/**
 * Copyright(C) 2017 Pedro H. Penna <pedrohenriquepenna@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/**
 * @brief NoC major device number.
 */
#define NOC_MAJOR 253

/**
 * @brief NoC minor device number.
 */
#define NOC_MINOR 0

/**
 * Buffer size.
 */
#define BUF_SIZE 16

/**
 * @brief NoC device filename.
 */
const char *devname = "/noc";

/**
 * @brief Panics the utility.
 */
static void panic(void)
{
	perror(NULL);
	exit(EXIT_FAILURE);
}

/**
 * @brief Initializes NoC device.
 */
static void init_noc(const char *pathname)
{
	int ret;   /* Return values of syscalls. */
	dev_t dev; /* NoC device number,         */

	/* Create device file. */
	dev = makedev(NOC_MAJOR, NOC_MINOR);
	ret = mknod(pathname, S_IFCHR | S_IRUSR | S_IWUSR, dev);
	if (ret != 0)
		panic();
}

int main(int argc, char **argv)
{
	int fd;   /* NoC device file descriptor. */
	int ret;  /* Return value of syscalls    */
	int buf[10]; /* Buffer for I/O operations.  */

	init_noc(devname);

	for (int i = 0; i < 10; i++)
		buf[i] = 10 - i;

	/* Open NoC device. */
	fd = ret = open(devname, O_RDWR);
	if (ret < 0)
		panic();

	/* Write some data. */
	ret = write(fd, buf, 10*sizeof(int));
	if (ret < 0)
		panic();

	/* Read some data. */
	ret = read(fd, buf, 10*sizeof(int));
	if (ret < 0)
		panic();

	/* Close NoC device. */
	close(fd);

	/* Stop here. */
	return (EXIT_SUCCESS);
}
