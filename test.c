/*
 * test.cpp
 *
 *  Created on: Jul 2, 2018
 *      Author: dmitry
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	printf("Test program!!!\n");
	int fd, retval = 0;

	if(argc > 1)
	{
		fd = open(argv[1], O_RDWR);
		if(fd < 0){
			printf("error: %m\n");
			close(fd);
			return 0;
		}

		write(fd, "12345", 5);
		sleep(15);
		close(fd);
	}
	else {
		printf("Argument error!\n");
	}
	return 0;
}
