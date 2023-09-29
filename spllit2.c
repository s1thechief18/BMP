#include <stdio.h>

int main() {
	FILE *ptr_readfile;
	FILE *ptr_writefile;
	char line [128]; /* or some other suitable maximum line size */
	char fileoutputname[15];
	int filecounter=1, linecounter=1;

	ptr_readfile = fopen("demo.txt","r");
	if (!ptr_readfile)
		return 1;

	sprintf(fileoutputname, "file_part%d", filecounter);
	ptr_writefile = fopen(fileoutputname, "w");

	while (fgets(line, sizeof line, ptr_readfile)!=NULL) {
		if (linecounter == 5) {
			fclose(ptr_writefile);
			linecounter = 1;
			filecounter++;
			sprintf(fileoutputname, "file_part%d", filecounter);
			ptr_writefile = fopen(fileoutputname, "w");
			if (!ptr_writefile)
				return 1;
		}
		fprintf(ptr_writefile,"%s\n", line);
		linecounter++;
	}
	fclose(ptr_readfile);
	return 0;
}