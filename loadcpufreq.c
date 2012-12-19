// (C) Curaga 2011
// Under the GPLv3

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define bufsiz 256

static void load(const char *mod) {
	pid_t pid = vfork();
	if (pid == 0) {
		execl("/sbin/modprobe", "/sbin/modprobe", "-b", mod, NULL);
	}
}

static char *dump(const char *buf) {
	char *tmp;

	tmp = strchr(buf, '\n');
	if (tmp) *tmp = '\0';

	tmp = strchr(buf, ':') + 2;
	return strdup(tmp);
}

static char *detect() {

	char *name = calloc(160, sizeof(char));
	if (!name) exit(1);

	sprintf(name, "acpi-cpufreq");

	FILE *cpuinfo = fopen("/proc/cpuinfo", "r");

	if (!cpuinfo) exit(1);

	char buf[bufsiz], *modelname = NULL, *vendorid = NULL;
	char *tmp;

	int modelid = 0, family = 0;

	while (fgets(buf, bufsiz, cpuinfo) != NULL) {
		if (strncmp("model name", buf, 10) == 0 && !modelname) {
			modelname = dump(buf);
		}

		else if (strncmp("model", buf, 5) == 0 && !modelid) {
			tmp = dump(buf);
			modelid = atoi(tmp);
			free(tmp);
		}

		else if (strncmp("vendor_id", buf, 9) == 0 && !vendorid) {
			vendorid = dump(buf);
		}

		else if (strncmp("cpu family", buf, 10) == 0 && !family) {
			tmp = dump(buf);
			family = atoi(tmp);
			free(tmp);
		}

	}
	rewind(cpuinfo);

	if (!modelname) modelname = calloc(1, sizeof(char));
	if (!vendorid) vendorid = calloc(1, sizeof(char));

/*	printf("model name %s\n", modelname);
	printf("model %d\n", modelid);
	printf("vendor id %s\n", vendorid);
	printf("family %d\n", family);*/


	if (strncmp("GenuineIntel", vendorid, 12) == 0) {
		while (fgets(buf, bufsiz, cpuinfo) != NULL) {
			if (strstr(buf, "est")) goto out;
		}

		if (family == 15 || strstr(modelname, " III ")) {
			sprintf(name, "speedstep-ich");
		}
	} else if (strncmp("AuthenticAMD", vendorid, 12) == 0) {
		switch(family) {
			case 5:
				sprintf(name, "powernow-k6");
			break;
			case 6:
				sprintf(name, "powernow-k7");
			break;
			case 15:
			case 16:
			case 17:
			case 20:
				sprintf(name, "powernow-k8");
			break;
		}
	} else if (strncmp("CentaurHauls", vendorid, 12) == 0) {
		if (family == 6) {
			switch (modelid) {
				case 10:
				break;
				default:
					sprintf(name, "longhaul");
				break;
			}
		}
	} else if (strncmp("GenuineTMx86", vendorid, 12) == 0) {
		while (fgets(buf, bufsiz, cpuinfo) != NULL) {
			if (strstr(buf, "longrun")) {
				sprintf(name, "longrun");
				break;
			}
		}
	}

	out:
	fclose(cpuinfo);
	free(modelname);
	free(vendorid);

	return name;
}

static void helpers(const char *ver) {

	char fp[PATH_MAX];
	snprintf(fp, PATH_MAX, "/lib/modules/%s/kernel/drivers/cpufreq", ver);

	if (chdir(fp)) exit(1);


	DIR *dir = opendir(".");
	if (!dir) exit(1);

	struct dirent *ent = readdir(dir);

	while (ent) {
		if (strncmp(ent->d_name, "cpufreq_", 8) == 0) {
//			printf("Loading %s\n", ent->d_name);
			load(ent->d_name);
		}

		ent = readdir(dir);
	}

	closedir(dir);
}

int main() {

	struct utsname un;

	if (uname(&un)) return 1;

	helpers(un.release);


	char *mod = detect();
//	printf("Loading %s\n", mod);
	load(mod);

	free(mod);

	return 0;
}
