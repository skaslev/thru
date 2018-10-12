#ifndef CMD_H_
#define CMD_H_

struct cmd {
	const char *name;
	int (*main)(int argc, char **argv);
	const char *usage;
};

#endif // CMD_H_
