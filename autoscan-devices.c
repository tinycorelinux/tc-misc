#include <stdio.h>
#include <mntent.h>
#include <string.h>

#define skipfile "/etc/init.d/tc_noscan.lst"
#define bufsiz 256

int main(){

	char *skips[50] = { NULL };
	FILE *f = fopen(skipfile,"r");
	if (!f) return 1;

	char buf[bufsiz];
	int i, j = 0;
	while (fgets(buf,bufsiz,f)) {
		i = 0;
		while (buf[i] != '\0') {
			if (buf[i] == '\n') buf[i] = '\0';
			i++;
		}
		skips[j] = strdup(buf);
		j++;
	}
	fclose(f);
	// skip list read. Proceed to process fstab.

	f = setmntent("/etc/fstab","r");
	struct mntent *ent;

	while ((ent = getmntent(f))) {
		if (strstr(ent->mnt_dir,"/mnt/") == NULL) continue;
		for (i=0; i<j; i++) {
			if (strstr(ent->mnt_dir,skips[i])) goto next;
			if (strstr(ent->mnt_type,skips[i])) goto next;
		} // skips done. Now just print the last part of mount path

		char *tmp = ent->mnt_dir, *tmp2 = NULL;
		for (i=0; tmp[i]; i++) if (tmp[i] == '/') tmp2=&tmp[i+1];
		printf("%s\n",tmp2);

		next:;
	}

	return 0;
}
