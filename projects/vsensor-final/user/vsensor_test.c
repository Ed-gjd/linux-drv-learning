// SPDX-License-Identifier: GPL-2.0-only
/*
 * vsensor_test — 用户态测试：阻塞读 5 个数 + ioctl GET_COUNT/RESET
 *
 * 编译：gcc -o vsensor_test vsensor_test.c
 * 用法：先 sudo insmod vsensor.ko，再 ./vsensor_test
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define VS_RESET     _IO('V', 1)
#define VS_GET_COUNT _IOR('V', 2, unsigned long)

#define DEV_PATH "/dev/vsensor"

int main(void)
{
	int fd, val, i;
	unsigned long count;

	fd = open(DEV_PATH, O_RDONLY);
	if (fd < 0) {
		perror("open /dev/vsensor");
		return 1;
	}

	printf("阻塞读 5 个数（每个采样间隔出一个）...\n");
	for (i = 0; i < 5; i++) {
		if (read(fd, &val, sizeof(val)) != sizeof(val)) {
			perror("read");
			close(fd);
			return 1;
		}
		printf("  读数[%d] = %d\n", i, val);
	}

	if (ioctl(fd, VS_GET_COUNT, &count) < 0) {
		perror("ioctl VS_GET_COUNT");
		close(fd);
		return 1;
	}
	printf("累计采样总数 = %lu\n", count);

	if (ioctl(fd, VS_RESET, 0) < 0) {
		perror("ioctl VS_RESET");
		close(fd);
		return 1;
	}
	printf("已执行 VS_RESET，环形缓冲清空\n");

	close(fd);
	return 0;
}
