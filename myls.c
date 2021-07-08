#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define DIRECTORY_SIZE MAXNAMLEN

void swap(struct stat* a, struct stat* b) {
	struct stat tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

void swapf(char* a, char* b) {
	char* tmp = (char*)malloc(sizeof(char)*255);
	strcpy(tmp, a);
	strcpy(a, b);
	strcpy(b, tmp);
	free(tmp);
}

void sortStat(char** files, struct stat* stats, int* iopt, int n) {
	for(int i = 0; i < n-1; i++) {
		int tmp = i;
		for(int j = i+1; j < n; j++) {
			if(stats[tmp].st_mtime < stats[j].st_mtime) {
				tmp = j;
			}
		}
		if(i != tmp) {
			swap(&stats[i], &stats[tmp]);
			swapf(files[i], files[tmp]);
			int itmp;
			itmp = iopt[i];
			iopt[i] = iopt[tmp];
			iopt[tmp] = itmp;
		}
	}
}

char fileType(mode_t m) {
	if(S_ISREG(m))
		return('-');
	else if(S_ISDIR(m))
		return ('d');
	else if(S_ISCHR(m))
		return ('c');
	else if(S_ISBLK(m))
		return ('b');
	else if(S_ISLNK(m))
		return ('l');
	else if(S_ISFIFO(m))
		return ('p');
	else if(S_ISSOCK(m))
		return ('s');
}

 char* perm(mode_t m) {
	int i;
	static char ret[10] = "---------";
	for(i = 0; i < 3; i++) {
		if(m & (S_IRUSR >> i*3))
			ret[i*3] = 'r';
		else
			ret[i*3] = '-';
		if(m & (S_IWUSR >> i*3))
			ret[i*3+1] = 'w';
		else
			ret[i*3+1] = '-';
		if(m & (S_IXUSR >> i*3))
			ret[i*3+2] = 'x';
		else
			ret[i*3+2] = '-';
	}
	if(m & S_ISUID) {
		if(m & S_IXUSR) 
			ret[2] = 's';
		else 
			ret[2] = 'S';
	}
	if(m & S_ISGID) {
		if(m & S_IXGRP) 
			ret[5] = 's';
		else 
			ret[5] = 'S';
	}
	if(m & S_ISVTX) {
		if(m & S_IXOTH)
			ret[8] = 't';
		else
			ret[8] = 'T';
	}
	return (ret);
}

void printStat(char** file, struct stat* stats, int* iopt, bool* opt, int n, int total) {
	if(opt[1] == 1)
		printf("total : %d\n", total/2);
	for(int i = 0; i < n; i++) {
		struct stat tmp = stats[i];
		if(opt[0] == 1) {
			printf("%7d ", iopt[i]);
		}
		if(opt[1] == 1) {
			printf("%c%s ", fileType(tmp.st_mode), perm(tmp.st_mode));
			printf("%3ld ", tmp.st_nlink);
			printf("%s %s ", getpwuid(tmp.st_uid)->pw_name, getgrgid(tmp.st_gid)->gr_name);
			printf("%9ld ", tmp.st_size);
			printf("%.12s ", ctime(&tmp.st_mtime)+4);
		}
		if(fileType(tmp.st_mode) == 'l') {
			char symfile[255] = {0};
			readlink(file[i], symfile, 255);
			printf("%s -> %s\n", file[i], symfile);
		}
		else {
			printf("%s\n", file[i]);
		}
	}
}

void ls(char* pathname, bool* opt) {
	struct dirent* entry;
	DIR* dirp;
	char filename[1024];
	struct stat stats[1024];
	char* files[1024];
	int iopt[1024];
	struct stat statbuf;
	int idx = 0;
	int total = 0;
	if((dirp = opendir(pathname)) == NULL || chdir(pathname) == -1) {
		fprintf(stderr, "opendir or chdir error | path : %s", pathname);
		exit(1);
	}
	while((entry = readdir(dirp)) != NULL) {
		if(entry->d_ino == 0) continue;
		strcpy(filename, entry->d_name);
		if(filename[0] == '.') continue;
		if(lstat(filename, &statbuf) == -1) {
			fprintf(stderr, "stat error\n");
			exit(1);
		}
		total += statbuf.st_blocks;
		files[idx] = (char*)malloc(sizeof(char)*255);
		strcpy(files[idx], filename);
		stats[idx] = statbuf;
		iopt[idx] = entry->d_ino;
		idx++;
	}
	if(opt[2] == 1) {
		sortStat(files, stats, iopt, idx);
	}
	printStat(files, stats, iopt, opt, idx, total);
	closedir(dirp);
}

void ls2(char* filename, bool* opt) {
	struct stat statbuf;
	char* pathname;
	pathname = malloc(1024);
	DIR* dirp;
	struct dirent* entry;
	int ino;
	bool sym = false;
	if(lstat(filename, &statbuf) == -1) {
		fprintf(stderr, "stat error\n");
		exit(1);
	}
	if(fileType(statbuf.st_mode) == 'd') {
		ls(filename, opt);
	}
	else {
		if(fileType(statbuf.st_mode) == 'l') {
			sym = true;
		}
		if(opt[0] == 1) {
			if(getcwd(pathname, 1024) == NULL) {
				fprintf(stderr, "getcwd error\n");
				exit(1);
			}
			if((dirp = opendir(pathname)) == NULL) {
				fprintf(stderr, "opendir error | path : %s", pathname);
				exit(1);
			}
			while((entry = readdir(dirp)) != NULL) {
				if(entry->d_ino == 0) continue;
				if(strcmp(entry->d_name, filename) == 0) {
					ino = entry->d_ino;
					break;
				}
			}
			printf("%d ", ino);
			closedir(dirp);
		}
		if(opt[1] == 1) {
			printf("%c%s ", fileType(statbuf.st_mode), perm(statbuf.st_mode));
			printf("%ld ", statbuf.st_nlink);
			printf("%s %s ", getpwuid(statbuf.st_uid)->pw_name, getgrgid(statbuf.st_gid)->gr_name);
			printf("%ld ", statbuf.st_size);
			printf("%.12s ", ctime(&statbuf.st_mtime) +4);
		}
		if(sym) {
			char symfile[255] = {0};
			readlink(filename, symfile, 255);
			printf("%s -> %s\n", filename, symfile);
		}
		else {
			printf("%s\n", filename);
		}
	}
}

int main(int argc, char** argv) {
	/*if(argc  3) {
		fprintf(stderr, "usage : %s [-ilt]: [file_name or directory_name]", argv[0]);
		exit(1);
	}*/
	char* pathname;
	pathname = malloc(1024);
	bool opt[3] = {0};
	struct stat statbuf;
	
	if(argc == 1) {
		if(getcwd(pathname, 1024) == NULL) {
			fprintf(stderr, "getcwd error\n");
			exit(1);
		}
		ls(pathname, opt);

	}
	else if(argc == 2) {
		if(argv[1][0] == '-') {
			for(int i = 1; i < strlen(argv[1]); i++) {
				char tmp = argv[1][i];
				if(tmp == 'i') opt[0] = 1;
				else if(tmp == 'l') opt[1] = 1;
				else if(tmp == 't') opt[2] = 1;
			}
		
			if(getcwd(pathname, 1024) == NULL) {
				fprintf(stderr, "getcwd error\n");
				exit(1);
			}
			ls(pathname, opt);
		}
		else {
			ls2(argv[1], opt);
		}
	}
	else {
		if(argv[1][0] == '-') {
			for(int i = 1; i < strlen(argv[1]); i++) {
				char tmp = argv[1][i];
				if(tmp == 'i') opt[0] = 1;
				else if(tmp == 'l') opt[1] = 1;
				else if(tmp == 't') opt[2] = 1;
			}
			for(int i = 2; i < argc; i++) {
				pathname = argv[i];
				ls2(pathname, opt);
			}
		}
		else {
			for(int i = 1; i < argc; i++) {
				ls2(argv[i], opt);
			}
		}
	}
	return 0;
}
