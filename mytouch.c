#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <stdbool.h>

int main(int argc, char** argv) {
	if(argc < 2) {
		fprintf(stderr, "usage : %s <file1> <file2> ...", argv[0]);
		exit(1);
	}
	bool opt = false;
	char* filename;
	int idx = 1;
	if(argv[1][0] == '-') {
		opt = true;
		idx++;
	}
	for(int i = idx; i < argc; i++) {
		filename = argv[i];
		if(access(filename, F_OK) < 0) {
			if(opt) continue;
			int fd;
			if((fd = creat(filename, 0664)) < 0) {
				fprintf(stderr, "file create error for %s\n", filename);
				exit(1);
			}
			close(fd);
		}
		else {
			if(utime(filename, NULL) < 0 ) {
				fprintf(stderr, "utime error\n");
				exit(1);
			}
		}
	}
}
