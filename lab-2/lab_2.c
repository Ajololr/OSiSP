#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

long count = 0;

int findFile(char dirName[], char targetFile[], char progName[])
{
	DIR *dir = opendir(dirName);
	struct stat St;

	if (!dir) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dirName);
		return 1;
	}
	struct dirent *dirStruct = readdir(dir);
	if (!dirStruct) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dirName);
		return 1;
	}
	while (dirStruct != NULL) {
		if ((strcmp(dirStruct->d_name, ".") == 0) ||
		(strcmp(dirStruct->d_name, "..") == 0)) {
			dirStruct = readdir(dir);
			continue;
		}
		count++;
		char* newDir = malloc(strlen(dirName) + strlen(dirStruct->d_name) + 2);
		sprintf(newDir, "%s/%s", dirName, dirStruct->d_name);
		if ((dirStruct->d_type == DT_REG) && !(strcmp(dirStruct->d_name,targetFile))) {
			if (stat(newDir, &St) == 0) {
				printf("%s %ldB %ld %d %ld\n", newDir, St.st_size, St.st_ctime, St.st_mode, St.st_ino);
				printf("Просмотренно: %ld\n", ++count);
			} else {
				fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), newDir);
				return 1;
			}
		} else if ((dirStruct->d_type) == DT_DIR) {
			findFile(newDir, targetFile, progName);
		}
		free(newDir);
		errno = 0;
		dirStruct = readdir(dir);
		if (!dirStruct && errno) {
			fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dirName);
			return 1;
		}
	}
	if(closedir(dir) != 0) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dirName);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char* progName = basename(argv[0]);
	if (argc < 3) {
		fprintf(stderr, "%s: Нужно передавать 2 аргумента.\n", progName);
		return 1;
	}
	findFile(argv[1], argv[2], progName);
	return 0;
}
