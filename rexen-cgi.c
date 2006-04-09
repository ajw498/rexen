/*
	$Id$
*/


#ifdef __riscos__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <unixlib/local.h>
#include <swis.h>

static void error(char *msg)
{
	printf("127\n%s\n", msg);
	exit(1);
}

int main(void)
{
	char *query;
	char *file;
	char riscosfile[PATH_MAX];
	char *dir;
	char binname[PATH_MAX];
	char stdinname[PATH_MAX];
	char stdoutname[PATH_MAX];
	char stderrname[PATH_MAX];
	char argsname[PATH_MAX];
	char cmd[PATH_MAX];
	char *rootdir;
	char *localdir;
	char *csd;
	FILE *fileh;

	printf("Content-Type: text/plain\n\n");

	query = getenv("QUERY_STRING");
	if (query == NULL) error("Cannot read QUERY_STRING");

	file = strstr(query, "&file=");
	dir = strstr(query, "dir=");

	if (file == NULL || dir == NULL || file < dir) error("File or dir not or incorrectly specified in query string");
	dir += 4;

	rootdir = getenv("REXEN$Remote");
	if (rootdir == NULL) {
		if (strncmp(dir, "/home/", 6) == 0) {
			char *slash = dir + 6;
			while (*slash && *slash != '/' && *slash != '&') slash++;
			rootdir = malloc(slash - dir + 1);
			if (rootdir == NULL) error("Out of memory");
			memcpy(rootdir, dir, slash - dir);
			rootdir[slash - dir] = '\0';
		} else {
			error("REXEN$Remote not specified");
		}
	}

	localdir = getenv("REXEN$Local");
	if (localdir == NULL) {
		if (strncmp(rootdir, "/home/", 6) == 0) {
			localdir = malloc(strlen(rootdir) - 6 + sizeof("Sunfish:Mounts.") + 1);
			if (localdir == NULL) error("Out of memory");
			sprintf(localdir, "Sunfish:Mounts.%s", rootdir + 6);
		} else {
			error("REXEN$Local not specified");
		}
	}

	if (chdir(localdir)) error("Cannot chdir");

	if (strncmp(rootdir, dir, strlen(rootdir)) != 0) {
		error("Dir not accessable over network");
	}
	dir += strlen(rootdir);
	while (dir[0] == '/') dir++;
	if (dir[0] != '\0') {
		memcpy(cmd, dir, file - dir);
		cmd[file - dir] = '\0';
		__riscosify_std(cmd, 0, riscosfile, PATH_MAX, NULL);
		if (chdir(riscosfile)) error("Cannot chdir");;
	}

	file += 6;
	__riscosify_std(file, 0, riscosfile, PATH_MAX, NULL);

	if (riscosfile[0] == '@') csd = ""; else csd = "@.";
	snprintf(stdinname, PATH_MAX, "%s%s-stdin", csd, riscosfile);
	snprintf(stdoutname, PATH_MAX, "%s%s-stdout", csd, riscosfile);
	snprintf(stderrname, PATH_MAX, "%s%s-stderr", csd, riscosfile);
	snprintf(binname, PATH_MAX, "%s%s-cross", csd, riscosfile);
	snprintf(argsname, PATH_MAX, "%s%s-args", csd, riscosfile);

	fileh = fopen(argsname, "r");
	if (fileh == NULL) error("Can't open args file");
	if (fgets(argsname, PATH_MAX, fileh) == NULL) argsname[0] = '\0';
	fclose(fileh);

	snprintf(cmd, PATH_MAX, "Run %s %s < %s > %s 2> %s", binname, argsname, stdinname, stdoutname, stderrname);

	_swix(Wimp_Initialise, _INR(0,3), 310, 0x4B534154, "rexen-cgi", 0);
	_swix(Wimp_StartTask, _IN(0), cmd);

	printf("%s\n",getenv("Sys$ReturnCode"));

	return 0;
}

#else

int main(void)
{
	return 1;
}

#endif
