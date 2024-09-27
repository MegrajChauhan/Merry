#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

int main(int argc, char **argv)
{
	int fd;
	printf("Hello World!\n");
	printf("%d\n", argc);
	if (argc > 1)
	{
		fd = strtol(argv[1], NULL, 10);
		printf("%d\n", fd);
	}
	else
	{
		int fd2[2];
		pipe(fd2);
		char val[2];
		snprintf(val, sizeof(val), "%d", fd2[1]);
		char *const c[] = {"./a.out", val, NULL};
		if (fork() == 0)
		{
			execv("./a.out", c);
		}
		close(fd2[1]);
		char msg[14];
		read(fd2[0], msg, sizeof(msg));
		printf("%s", msg);
		close(fd2[0]);
		return 0;
	}
	// try writing
	char c[] = "Hello World!\n";
	write(fd, c, sizeof(c));
	close(fd);
}
