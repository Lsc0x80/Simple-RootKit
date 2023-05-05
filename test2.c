#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main() {
	system("ls");
	system("ps");
	char cmd[128];
	sprintf(cmd, "cat /proc/%d/stat", getpid());
	system(cmd);
}
