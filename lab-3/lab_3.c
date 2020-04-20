#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <sys/resource.h>

int maxProcessCount, currentCount = 1;
char* progName;

long File_Copy (char FileSource [], char FileDestination [], mode_t FileMod)
{
    int   c;
    FILE *stream_R;
    FILE *stream_W; 

    stream_R = fopen (FileSource, "r");
    if (stream_R == NULL)
        return -1;
    stream_W = fopen (FileDestination, "w");
    if (stream_W == NULL)
     {
        fclose (stream_R);
        return -2;
     }  
    long count = 0;  
    while ((c = fgetc(stream_R)) != EOF) {
        fputc (c, stream_W);
        count++;
    }
    fclose (stream_R);
    fclose (stream_W);
    chmod(FileDestination, FileMod);
    return count;
}

int containsFileOrDir(char* name) {
    struct stat st;
    if(stat(name, &st) == 0) {
        return 1;
    } else if (access(name, F_OK) != -1) {
        return 1;
    }
    return 0;
}

int copyDir(char* sorce, char* dest) {
	DIR *src = opendir(sorce);
	DIR *dst = opendir(dest);

	if (!src) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), sorce);
		return 1;
	}
    if (!dst) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dest);
		return 1;
	}
	struct dirent *dirStruct = readdir(src);
	if (!dirStruct) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), sorce);
		return 1;
	}
	while (dirStruct != NULL) {
        // puts(dirStruct->d_name);
		if ((strcmp(dirStruct->d_name, ".") == 0) ||
		(strcmp(dirStruct->d_name, "..") == 0)) {
			dirStruct = readdir(src);
			continue;
		}
		char* newDir = malloc(strlen(sorce) + strlen(dirStruct->d_name) + 2);
		char* checkDir = malloc(strlen(dest) + strlen(dirStruct->d_name) + 2);
		sprintf(newDir, "%s/%s", sorce, dirStruct->d_name);
		sprintf(checkDir, "%s/%s", dest, dirStruct->d_name);
		if (!containsFileOrDir(checkDir)) {
            // printf("Файл %s не содержится в %s\n", newDir, dest);
            int retCode;
            if( currentCount == maxProcessCount ) {
                wait(&retCode);
                currentCount--;
            }
            pid_t pid = fork();
            if (pid == 0) {
                pid = getpid();
                printf("%d %s ", pid, checkDir);
                struct stat St;
                stat(newDir, &St);
                if (dirStruct->d_type != DT_DIR) {
                    long count = File_Copy(newDir, checkDir, St.st_mode);
                    printf("%ld\n", count);
                }
                free(newDir);
                free(checkDir);
                exit(0);
            } else if (pid > 0) {
                currentCount++;
                if ((dirStruct->d_type == DT_DIR)) {
                    // wait(NULL);
                    struct stat St;
                    stat(newDir, &St);
                    int retval = mkdir(checkDir, St.st_mode);
                    if (retval) {
                        fprintf(stderr, "%s: %s %s\n", progName, "не удалось создать папку", newDir);
                        return 1;
                    }
                    copyDir(newDir, checkDir);
                }
                // pid_t cpid = wait(NULL);
            } else {
                fprintf(stderr, "%s: %s\n", progName, "процесс не создан.");
                return 1;
            }
		} else {
            if ((dirStruct->d_type == DT_DIR)) {
                copyDir(newDir, checkDir);
            }
        }
        free(newDir);
        free(checkDir);
        errno = 0;
        dirStruct = readdir(src);
        if (!dirStruct && errno) {
            fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), sorce);
            return 1;
        }
	}
	if(closedir(src) != 0) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), sorce);
	}
	if(closedir(dst) != 0) {
		fprintf(stderr, "%s: %s %s\n", progName, strerror(errno), dest);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	progName = basename(argv[0]);
	if (argc < 4) {
		fprintf(stderr, "%s: Нужно передавать 3 аргумента.\n", progName);
		return 1;
	}
    maxProcessCount = atoi(argv[3]);
	copyDir(argv[1], argv[2]);
    while(wait(NULL) != -1){}
	return 0;
}

/* Написать программу синхронизации двух каталогов Dir1 и Dir2
вместе с правами доступа, для файлов, отслутствующих в яDir2
каждый процесс выводит на экран свой пид, полный путь к файлу
 и число скопированных байт, максимальное количество , число одновременно работающих процессов не должно превышат N
*/