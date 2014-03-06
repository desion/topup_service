#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdint.h>

#define CONF_LINE_LEN_MAX 1024

int conf_read_string(const char *filepath, const char *name, char *valuebuf,
                     int value_buf_size)
{
    FILE *fp = NULL;
    char line[CONF_LINE_LEN_MAX];
    char namebuf[CONF_LINE_LEN_MAX];
    char sep[CONF_LINE_LEN_MAX];
    char _valuebuf[CONF_LINE_LEN_MAX];

    if ((fp = fopen(filepath, "r")) == NULL)
        return -1;

    for (;;) {
        if (!fgets(line, CONF_LINE_LEN_MAX, fp)) {
            if (feof(fp))
                break;
            else
                return -1;
        }
        char *ptr = line;
        while (isspace(*ptr))
            ptr++;

        if (*ptr != '#') {
            if (sscanf(ptr, "%s%s%s", namebuf, sep, &_valuebuf) == 3) {
                if (strcmp(namebuf, name) == 0 &&
                    (strcmp(sep, ":") == 0 || strcmp(sep, "=") == 0)) {
                    strncpy(valuebuf, _valuebuf, value_buf_size);
                    fclose(fp);
                    return 1;
                }
            }
        }
    }

    fclose(fp);
    return 0;
}

int conf_read_int32(const char *filepath, const char *name,
                    int32_t * valuebuf)
{
    char _valuebuf[CONF_LINE_LEN_MAX];
    int rtn = conf_read_string(filepath, name, _valuebuf, CONF_LINE_LEN_MAX);
    if (rtn <= 0)
        return rtn;

    return sscanf(_valuebuf, "%d", valuebuf);
}

int conf_read_uint32(const char *filepath, const char *name,
                     uint32_t * valuebuf)
{
    char _valuebuf[CONF_LINE_LEN_MAX];
    int rtn = conf_read_string(filepath, name, _valuebuf, CONF_LINE_LEN_MAX);
    if (rtn <= 0)
        return rtn;

    return sscanf(_valuebuf, "%u", valuebuf);
}

int conf_read_int64(const char *filepath, const char *name,
                    int64_t * valuebuf)
{
    char _valuebuf[CONF_LINE_LEN_MAX];
    int rtn = conf_read_string(filepath, name, _valuebuf, CONF_LINE_LEN_MAX);
    if (rtn <= 0)
        return rtn;

    return sscanf(_valuebuf, "%lld", valuebuf);
}

#ifdef TESTCONF
int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "%s <filepath>\n", argv[0]);
		exit(1);
	}
	int i = 0;
	int rtn = conf_read_int32(argv[1], "curr", &i);
	fprintf(stdout, "rtn [%d], i = %d\n", rtn, i);
	exit(0);
}
#endif
