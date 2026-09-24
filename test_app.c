/*
 * test_app.c - A normal user-space program that talks to /dev/hello
 *
 * Build:  gcc test_app.c -o test_app
 * Run:    sudo ./test_app "Hello from user space"
 */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>	/* open */
#include <unistd.h>	/* read, write, close */

int main(int argc, char *argv[])
{
	const char *msg = (argc > 1) ? argv[1] : "Hello, kernel!";
	char buf[256] = {0};
	ssize_t n;

	/* 1. open() -> system call -> kernel -> hello_open() */
	int fd = open("/dev/hello", O_RDWR);
	if (fd < 0) {
		perror("open /dev/hello");
		return 1;
	}

	/* 2. write() -> system call -> kernel -> hello_write() */
	n = write(fd, msg, strlen(msg));
	printf("Wrote %zd bytes : \"%s\"\n", n, msg);

	/* 3. Go back to the start of the file, then read it back */
	lseek(fd, 0, SEEK_SET);

	/* 4. read() -> system call -> kernel -> hello_read() */
	n = read(fd, buf, sizeof(buf) - 1);
	printf("Read  %zd bytes : \"%s\"\n", n, buf);

	/* 5. close() -> system call -> kernel -> hello_release() */
	close(fd);
	return 0;
}
