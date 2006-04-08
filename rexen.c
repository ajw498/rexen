/*
	$Id$
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

static char *progname;

void error(char *msg)
{
	fprintf(stderr, "%s: %s\n", progname, msg);
	exit(127);
}

int main(int argc, char *argv[])
{
	char *nativebinname;
	char stdinname[PATH_MAX];
	char stdoutname[PATH_MAX];
	char stderrname[PATH_MAX];
	char curloutputname[PATH_MAX];
	char binlinkname[PATH_MAX];
	char cmd[PATH_MAX];
	FILE *file;
	int returncode;
	int ch;

	progname = argv[0];

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <binary> [args]\n", progname);
		return 127;
	}

	nativebinname = argv[1];

	snprintf(stdinname, PATH_MAX, "%s-stdin", nativebinname);
	snprintf(stdoutname, PATH_MAX, "%s-stdout", nativebinname);
	snprintf(stderrname, PATH_MAX, "%s-stderr", nativebinname);
	snprintf(curloutputname, PATH_MAX, "%s-curlout", nativebinname);
	snprintf(binlinkname, PATH_MAX, "%s-cross,ff8", nativebinname);

	unlink(stdoutname);
	unlink(stderrname);
	unlink(curloutputname);

	/* Ignore error incase link already exists */
	returncode = symlink(nativebinname, binlinkname);

	/* Dump stdin to file */
	file = fopen(stdinname, "w");
	if (file == NULL) error("Can't open stdin file");
	if (!isatty(fileno(stdin))) {
		while ((ch = fgetc(stdin)) != EOF) fputc(ch, file);
	}
	fclose(file);

	/* Call the cgi to run the cross prog */
	snprintf(cmd, PATH_MAX, "curl --fail --silent --show-error $REXEN_CGI?dir=`pwd`\\&file=%s > %s",nativebinname, curloutputname);
	returncode = system(cmd);

	if (returncode == 0) {
		/* Read curl output file which contains return code */
		file = fopen(curloutputname, "r");
		if (file == NULL) error("Can't open curl output file");

		if (fgets(cmd, PATH_MAX, file) == NULL) error("Can't read curl output file");
		if (cmd[0] == '\0' || (cmd[0] == '\n' && cmd[1] == '\0')) {
			if (fgets(cmd, PATH_MAX, file) == NULL) error("Can't read return code from curl output file");
		}
		returncode = strtol(cmd, NULL, 10);
		if (fgets(cmd, PATH_MAX, file)) error(cmd);
		fclose(file);

		/* Output stderr */
		file = fopen(stderrname, "r");
		if (file == NULL) error("Can't open stderr file");
		while ((ch = fgetc(file)) != EOF) fputc(ch, stderr);
		fclose(file);

		/* Output stdout */
		file = fopen(stdoutname, "r");
		if (file == NULL) error("Can't open stdout file");
		while ((ch = fgetc(file)) != EOF) fputc(ch, stdout);
		fclose(file);
	}

	unlink(stdinname);
	unlink(stdoutname);
	unlink(stderrname);
	unlink(curloutputname);
	unlink(binlinkname);

	return returncode;
}
