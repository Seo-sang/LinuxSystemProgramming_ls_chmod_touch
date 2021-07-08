#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

void setMode(struct stat* st, bool* tmpperm, int m, int who, bool* bit);

int main(int argc, char** argv) {
	if(argc < 3) {
		fprintf(stderr, "usage : %s <permission> <file>\n", argv[0]);
		exit(1);
	}
	for(int i = 2; i < argc; i++) {
		char filename[100] = {0}; 
		strcpy(filename, argv[i]);
		struct stat st;
		bool perm[3][3] = {0};
		int m;
		if(access(filename, F_OK) < 0) {
			fprintf(stderr, "%s file does not exist", filename);
			exit(1);
		}
		if(stat(filename, &st) < 0) {
			fprintf(stderr, "stat error\n");
			exit(1);
		}

		if(argv[1][0] >= '0' && argv[1][0] <= '9') {
			int k = 0;
			int tmp;
			bool bit[3] = {0};
			if(strlen(argv[1]) == 4) {
				tmp = argv[1][0] - '0';
				if(tmp == 4) {
					st.st_mode |= S_ISUID;
					st.st_mode &= ~S_ISGID;
					st.st_mode &= ~S_ISVTX;
				}
				else if(tmp == 2) {
					st.st_mode &= ~S_ISUID;
					st.st_mode |= S_ISGID;
					st.st_mode &= ~S_ISVTX;
				}
				else if(tmp == 1) {
					st.st_mode &= ~S_ISUID;
					st.st_mode &= ~S_ISGID;
					st.st_mode |=S_ISVTX;
				}

				k++;
			}
			else {
				st.st_mode &= ~S_ISUID;
				st.st_mode &= ~S_ISGID;
				st.st_mode &= ~S_ISVTX;
			}
			for(int i = k; i < strlen(argv[1]); i++) {
				bool tmpperm[3] = {0};
				tmp = argv[1][i] - '0';
				for(int j = 0; j < 3; j++) {
					if(tmp & (1 << j)) {

						tmpperm[2-j] = true;
					}
				}
				if(strlen(argv[1]) == 4)
					setMode(&st, tmpperm, 3, i-1, bit);
				else
					setMode(&st, tmpperm, 3, i, bit);
			}
		}
		else {
			int idx = 0;

			bool perm[3][3] = {0};
			bool who[3] = {0};
			bool bit[3] = {0};
			for(int i = 0; i < strlen(argv[1]); i++) {
				char tmp = argv[1][i];
				if(tmp == 'a') {
					for(int j = 0; j < 3; j++) {
						who[j] = true;
					}
				}
				if(tmp == 'u') {
					who[0] = true;
					continue;
				}
				if(tmp == 'g') {
					who[1] = true;
					continue;
				}
				if(tmp == 'o') {
					who[2] = true;
					continue;
				}
				if(tmp == '+') {
					m = 1;
					if(!(who[0] || who[1] || who[2])) {
						for(int j = 0; j < 3; j++) {
							who[j] = true;
						}
					}
					continue;
				}
				if(tmp == '-') {
					if(!(who[0] || who[1] || who[2])) {
						for(int j = 0; j < 3; j++) {
							who[j] = true;
						}
					}
					m = 2;
					continue;
				}
				if(tmp == '=') {
					if(!(who[0] || who[1] || who[2])) {
						for(int j = 0; j < 3; j++) {
							who[j] = true;
						}
					}
					m = 3;
					continue;
				}
				if(tmp == 'r') {
					for(int j = 0;  j < 3; j++) {
						if(who[j]) {
							perm[j][0] = true;
						}
					}
					continue;
				}
				if(tmp == 'w') {
					for(int j = 0; j < 3; j++) {
						if(who[j]) {
							perm[j][1] = true;
						}
					}
					continue;
				}
				if(tmp == 'x') {
					for(int j = 0; j < 3; j++) {
						if(who[j]) {
							perm[j][2] = true;
						}
					}
					continue;
				}
				if(tmp == 's') {
					for(int j = 0; j < 2; j++) {
						if(who[j]) {
							bit[j] = true;
						}
					}
					continue;
				}
				if(tmp == 't') {
					if(who[2]) {
						bit[2] = true;
					}
					continue;
				}
				if(tmp == ',') {
					for(int j = 0; j < 3; j++) {
						if(who[j]) {
							setMode(&st, perm[j], m, j, bit);
						}
					}
					memset(perm, 0, sizeof(perm));
					memset(who, 0, sizeof(who));
					memset(bit, 0, sizeof(bit));
					continue;
				}
				else {
					fprintf(stderr, "<permission> usage : [u/g/o][+/-][r/w/x]\n");
					exit(1);
				}
			}
			for(int j =0; j < 3; j++) {
				if(who[j]) {
					setMode(&st, perm[j], m, j, bit);
				}
			}
		}

		if(chmod(filename, st.st_mode) < 0) {
			fprintf(stderr, "chmod error\n");
			exit(1);
		}
	}
}


void setMode(struct stat* st,bool* tmpperm, int m, int who, bool* bit) {
	if(m == 1) {
		if(bit[0]) {
			st->st_mode |= S_ISUID;
		}
		if(bit[1]) {
			st->st_mode |= S_ISGID;
		}
		if(bit[2]) {
			st->st_mode |= S_ISVTX;
		}
		if(who == 0) {
			if(tmpperm[0]) {
				st->st_mode |= S_IRUSR;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWUSR;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXUSR;
			}
			return;
		}
		else if(who == 1) {
			if(tmpperm[0]) {
				st->st_mode |= S_IRGRP;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWGRP;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXGRP;
			}
			return;
		}
		else if(who == 2) {
			if(tmpperm[0]) {
				st->st_mode |= S_IROTH;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWOTH;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXOTH;
			}
			return;
		}
	}
	else if(m == 2){
		if(bit[0])
			st->st_mode &= ~S_ISUID;
		if(bit[1])
			st->st_mode &= ~S_ISGID;
		if(bit[2])
			st->st_mode &= ~S_ISVTX;
		if(who == 0) {
			if(tmpperm[0]) {
				st->st_mode &= ~S_IRUSR;
			}
			if(tmpperm[1]) {
				st->st_mode &= ~S_IWUSR;
			}
			if(tmpperm[2]) {
				st->st_mode &= ~S_IXUSR;
			}
			return;
		}
		else if(who == 1) {
			if(tmpperm[0]) {
				st->st_mode &= ~S_IRGRP;
			}
			if(tmpperm[1]) {
				st->st_mode &= ~S_IWGRP;
			}
			if(tmpperm[2]) {
				st->st_mode &= ~S_IXGRP;
			}
			return;
		}
		else if(who == 2) {
			if(tmpperm[0]) {
				st->st_mode &= ~S_IROTH;
			}
			if(tmpperm[1]) {
				st->st_mode &= ~S_IWOTH;
			}
			if(tmpperm[2]) {
				st->st_mode &= ~S_IXOTH;
			}
			return;
		}
	}	
	else if(m == 3) {
		if(bit[0])
			st->st_mode |= S_ISUID;
		if(bit[1])
			st->st_mode |= S_ISGID;
		if(bit[2])
			st->st_mode |= S_ISVTX;
		if(who == 0) {
			if(tmpperm[0]) {
				st->st_mode |= S_IRUSR;
			}
			else {
				st->st_mode &= ~S_IRUSR;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWUSR;
			}
			else {
				st->st_mode &= ~S_IWUSR;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXUSR;
			}
			else {
				st->st_mode &= ~S_IXUSR;
			}
			return;
		}
		else if(who == 1) {
			if(tmpperm[0]) {
				st->st_mode |= S_IRGRP;
			}
			else {
				st->st_mode &= ~S_IRGRP;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWGRP;
			}
			else {
				st->st_mode &= ~S_IWGRP;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXGRP;
			}
			else {
				st->st_mode &= ~S_IXGRP;
			}
			return;
		}
		else if(who ==2) {
			if(tmpperm[0]) {
				st->st_mode |= S_IROTH;
			}
			else {
				st->st_mode &= ~S_IROTH;
			}
			if(tmpperm[1]) {
				st->st_mode |= S_IWOTH;
			}
			else {
				st->st_mode &= ~S_IWOTH;
			}
			if(tmpperm[2]) {
				st->st_mode |= S_IXOTH;
			}
			else {
				st->st_mode &= ~S_IXOTH;
			}
			return;
		}
	}
}
