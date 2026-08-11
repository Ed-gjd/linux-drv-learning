// SPDX-License-Identifier: GPL-2.0-only
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* 命令号必须和驱动里完全一致 */
#define HELLODEV_CLEAR  _IO('H', 1)
#define HELLODEV_GETLEN _IOR('H', 2, int)

int main(void)
{
	int fd, len;

	fd = open("/dev/hellodev", O_RDWR);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	write(fd, "abcdef", 6);                 /* 先写6字节 */
	ioctl(fd, HELLODEV_GETLEN, &len);       /* 问长度 */
	printf("写入6字节后 GETLEN = %d\n", len);

	ioctl(fd, HELLODEV_CLEAR);              /* 清空 */
	ioctl(fd, HELLODEV_GETLEN, &len);       /* 再问长度 */
	printf("CLEAR 后 GETLEN = %d\n", len);

	close(fd);
	return 0;
}
