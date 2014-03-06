
 
/**
 * @file ul_file.cpp
 * @date 2007/12/09 14:16:18
 * @version $Revision: 1.5 $ 
 * @brief 文件处理库 
 *  
 **/
#include <stdio.h>
#include <fcntl.h>

#include "ul_def.h"
#include "ul_file.h"
#include "ul_func.h"
#include "ul_conf.h"
#include "slog.h"

#define TBUF_SIZE	(64*1024)

static int kreadnum_one(const char *work_path, const char *fname, char *vname, void *pvalue, int type)
{
    char fnum[PATH_SIZE];
    int reti = 0;
    ul_confdata_t *tconf;

    snprintf(fnum, sizeof(fnum), "%s.n", fname);

    tconf = ul_initconf(LIMIT_CONF_ITEM_NUM);

    if (ul_readconf(work_path, fnum, tconf) == -1) {
        ul_freeconf(tconf);
        return -1;
    }

    switch (type) {
        case ULTYPE_INT:
            reti = ul_getconfint(tconf, vname, (int *) pvalue);
            break;
        case ULTYPE_UINT:
            reti = ul_getconfuint(tconf, vname, (u_int *) pvalue);
            break;
        case ULTYPE_INT64:
            reti = ul_getconfint64(tconf, vname, (long long *) pvalue);
            break;
        case ULTYPE_UINT64:
            reti = ul_getconfuint64(tconf, vname, (unsigned long long *) pvalue);
            break;
        case ULTYPE_FLOAT:
            reti = ul_getconffloat(tconf, vname, (float *) pvalue);
            break;
    }

    ul_freeconf(tconf);

    if (reti) {
        return reti;
    } else {
        slog_write(LL_FATAL, "[kreadnum_one] have no (%s).", vname);
        return -1;
    }
}

int ul_fullpath(const char *path, const char *fname, char *fullpath, size_t size)
{
	if (NULL == fullpath) {
		return -1;
	}
	fullpath[0] = '\0';
    if (NULL == path || NULL == fname || 0 == size)
        return -1; 
    size_t path_len = strlen(path);
    int ret = 0;


    if (path_len > 0) {
        path_len --; 
        while(path_len > 0 && '/' == path[path_len]) {
            path_len--;
        }    
        if (path_len >= size) { 
            return -1; 
        }    
        memcpy(fullpath, path, path_len+1);
    } else {
        if (fname[0] != '/') {
            ret = snprintf(fullpath, size, "%s", fname);
            if ((size_t)ret >= size) {
                return -1; 
            }   
            return 0;
        }    
        fullpath[0] = '/';
    }   

    while (*fname != '\0' && '/' == *fname) {
        fname++;
    }

    if ('/' == fullpath[path_len]) {
        ret = snprintf(fullpath + path_len + 1, size - path_len - 1, "%s", fname);
    } else {
        ret = snprintf(fullpath + path_len + 1, size - path_len - 1, "/%s", fname);
    }
    if (ret + path_len + 1 >= size) {
        return -1;
    }

    return 0;
}

int ul_fexist(const char *path, const char *fname)
{
	char full_path[PATH_SIZE];
	struct stat statbuf;
	if (ul_fullpath(path, fname, full_path, sizeof(full_path)) < 0) {
		return 0;
	}

	if (stat(full_path, &statbuf) == -1) {
		return 0;
	}
	if(!S_ISREG(statbuf.st_mode)) {
		return 0;
	}
	return 1;
}

off_t ul_fsize(const char *path, const char *fname)
{
	char full_path[PATH_SIZE];
	struct stat statbuf;
	if (ul_fullpath(path, fname, full_path, sizeof(full_path)) < 0) {
		return -1;
	}
	if (stat(full_path,&statbuf) == -1) {
		return -1;
	}
	if(!S_ISREG(statbuf.st_mode)) {
		return -1;
	}
	return statbuf.st_size;
}

int ul_finfo(const char *path, const char *fname, int *nline, int *nword, int *nchar)
{
	char 	full_path[PATH_SIZE];
	int 	fno;
	unsigned char buf[TBUF_SIZE + 1];
	char	eng_map[256];
	ssize_t b_size;
	int i;
	int	nl, nw, nc;

	if (ul_fullpath(path, fname, full_path, sizeof(full_path)) < 0) {
		return -1;
	}

	/* check file exist */
	if (!ul_fexist(path, fname)) {
		return -1;
	}

	for (i = 0; i < 256; i++) {
		eng_map[i] = 1;
	}

	eng_map[32] = 2;
	eng_map[9] = 2;
	eng_map[13] = 2;
	eng_map[10] = 4;
	eng_map[0] = 8;

	/* open file */
	fno = ul_open(full_path, O_RDONLY, 0776);
	if (fno < 0) {
		return -1;
	}
	/* calc */
	nl = nw = nc = 0;

	while (1) {

		/* read buf */
		b_size = ul_read (fno, buf, TBUF_SIZE);
		if (b_size < 0) {
			return -1;
		}
		buf[b_size] = 0;
		if (0 == b_size) break;

		i = 0;
		while (1) {
			while(eng_map[buf[i]] & 1) {
				i++;
			}

new_line:
			while(eng_map[buf[i]] & 2) {
				i++;
			}
			if (eng_map[buf[i]] & 4) {
				i++;
				nl++;
				goto new_line;
			}
			nw++;

			if (eng_map[buf[i]] & 8) {
				break;
			}
		}
		nc+=b_size;

		if (b_size<TBUF_SIZE) {
			if (!(eng_map[buf[i-1]]&4)) {
				nl++;
			}
			break;
		}
		else{
			if (eng_map[buf[i-1]] & 1) {
				nw--;
			}
		}
	}

	/* close file */
	ul_close(fno);

	/* return */
	*nline = nl;
	*nword = nw;
	*nchar = nc;
	return 1;
}

int ul_nullfile(const char *path, const char *fname)
{
	char full_path[PATH_SIZE];
	FILE *pf;
	
    if (ul_fullpath(path, fname, full_path, sizeof(full_path)) < 0) {
		return -1;
	}
	pf = ul_fopen(full_path, "w");
	if (NULL == pf){
		slog_write(LL_FATAL, "[nullfile] open file (%s) error", full_path);
		return -1;
	}

	ul_fclose(pf);
	return 1;
}

int ul_cpfile(const char *path, const char *fsrc, const char *fdest)
{
	int pf_src, pf_dest;
	char filename[PATH_SIZE];
	char buf[TBUF_SIZE];
	ssize_t  real_read;
	
	if (ul_fullpath(path, fsrc, filename, sizeof(filename)) < 0) {
		return -1;
	}

	if (!ul_fexist(path, fsrc)) {
		slog_write(LL_FATAL,"[cpfile] file : %s do not exist.\n", fsrc);
		return -1;
	}

	pf_src = ul_open(filename, O_RDONLY,0776);
	if (pf_src < 0) {
		slog_write(LL_FATAL, "[cpfile] open %s error", fsrc);
		return -1;
	}
	
	if (ul_fullpath(path, fdest, filename, sizeof(filename)) < 0) {
		return -1;
	}
	pf_dest= ul_open(filename, O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if (pf_dest < 0) {
		ul_close(pf_src);
		slog_write(LL_FATAL, "[cpfile] open %s error", fdest);
		return -1;
	}

	while ((real_read = ul_read(pf_src, buf, TBUF_SIZE)) == TBUF_SIZE) {
		ul_write(pf_dest, buf, TBUF_SIZE);
	}
	ul_write(pf_dest, buf, (size_t)real_read);

	ul_close(pf_src);
	ul_close(pf_dest);

	return 1;
}


int ul_appfile(const char *path, const char *fsrc, const char *fdest)
{
	int pf_src, pf_dest;
	char filename[PATH_SIZE];
	char buf[TBUF_SIZE];
	ssize_t  real_read;

	if (ul_fullpath(path, fsrc, filename, sizeof(filename)) < 0) {
		return -1;
	}

	if (!ul_fexist(path, fsrc)) {
		slog_write(LL_FATAL, "[appfile] file : %s do not exist.\n", fsrc);
		return -1;
	}

	pf_src = ul_open(filename, O_RDONLY,0776);
	if (pf_src < 0) {
		slog_write(LL_FATAL,"[appfile] open %s error", fsrc);
		return -1;
	}

	if (ul_fullpath(path, fdest, filename, sizeof(filename)) < 0) {
		return -1;
	}

	pf_dest= ul_open(filename, O_WRONLY|O_APPEND|O_CREAT, 0666);
	if (pf_dest < 0) {
		ul_close(pf_src);
		slog_write(LL_FATAL, "[appfile] open %s error", fdest);
		return -1;
	}

	while((real_read=ul_read(pf_src, buf, TBUF_SIZE)) == TBUF_SIZE) {
		ul_write(pf_dest, buf, TBUF_SIZE);
	}
	ul_write(pf_dest, buf, (size_t)real_read);

	ul_close(pf_src);
	ul_close(pf_dest);

	return 1;
}


int ul_mvfile(const char *path, const char *fsrc, const char *fdest)
{
	int  reti;
	char ffsrc[PATH_SIZE], ffdest[PATH_SIZE];
	if (ul_fullpath(path, fsrc, ffsrc, sizeof(ffsrc)) < 0) {
		return -1;
	}
	if (ul_fullpath(path, fdest, ffdest, sizeof(ffdest)) < 0) {
		return -1;
	}

	reti = rename(ffsrc, ffdest);
	if (reti < 0) {
		slog_write(LL_FATAL, "[mvfile] rename (%s) to (%s) error", ffsrc, ffdest);
		ul_cpfile(path, fsrc, fdest);
		reti = unlink(ffsrc);
		if (reti < 0) {
			slog_write(LL_FATAL, "[mvfile] unlink file (%s) error", ffsrc);
			return -1;
		}
	}
	return reti;
}

ssize_t ul_readfile(const char *path, const char *fname, void *buf, int size)
{
	int fno ;
	ssize_t rsize;
	char fullname[PATH_SIZE];

	if (ul_fullpath(path, fname, fullname, sizeof(fullname)) < 0) {
		return -1;
	}

	fno = ul_open(fullname, O_RDONLY,0776);
	if (fno < 0) {
		slog_write(LL_FATAL, "[readfile] open (%s) error", fullname);
		return -1;
	}

	rsize = ul_read(fno, buf, size);
	if (rsize < 0) {
		slog_write(LL_FATAL, "[readfile] read (%s) error", fullname);
	}
	ul_close(fno);

	return rsize;
}


ssize_t ul_writefile(const char *path, const char *fname, void *buf, int size)
{
	int fno;
	ssize_t rsize;
	char fullname[PATH_SIZE];
	
    if (ul_fullpath(path, fname, fullname, sizeof(fullname)) < 0) {
		return -1;
	}
	fno = ul_open(fullname, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	fchmod (fno, 0666);
	if (fno < 0) {
		slog_write(LL_FATAL, "[writefile] open (%s) error", fullname);
		return -1;
	}
	rsize = ul_write(fno, buf, size);
	if (rsize < 0){
		slog_write(LL_FATAL, "[writefile] write(%s) error", fullname);
	}
	ul_close(fno);

	return rsize;
}

int ul_exchangedir(const char *path, const char *sname, const char *dname)
{
	char fsname[PATH_SIZE], fdname[PATH_SIZE], ftname[PATH_SIZE];

	if (ul_fullpath(path, sname, fsname, sizeof(fsname)) < 0) {
		return -1;
	}
	if (ul_fullpath(path, dname, fdname, sizeof(fdname)) < 0) {
		return -1;
	}
	if (ul_fullpath(path, "tmp.xxx", ftname, sizeof(ftname)) < 0) {
		return -1;
	}

	if (rename(fdname, ftname) < 0) {
		slog_write(LL_FATAL, "[exchangedir] rename(%s) to (%s) error", fdname, ftname);
		return -1;
	}

	if (rename(fsname, fdname) < 0) {
		slog_write(LL_FATAL, "[exchangedir] rename(%s) to (%s) error", fsname, fdname);
		return -1;
	}

	if (rename(ftname, fsname) < 0) {
		slog_write(LL_FATAL, "[exchangedir] rename(%s) to (%s) error", ftname, fsname);
		return -1;
	}
	return 1;
}

int ul_readnum_oneuint(const char *work_path, const char *fname, char *vname, u_int *pvalue)
{
	int ret;
	char fnum[PATH_SIZE];
	ul_confdata_t *tconf;
	sprintf(fnum, "%s.n", fname);
	tconf = ul_initconf(1024);
	if (ul_readconf(work_path, fnum, tconf) == -1) {
		ul_freeconf(tconf);
		return 0;
	}
	ret = ul_getconfuint(tconf, vname, pvalue);
	ul_freeconf(tconf);
	return ret;
}

int ul_readnum_oneint64(const char *work_path, const char *fname, char *vname, long long *pvalue)
{
	int ret;
	char fnum[PATH_SIZE];
	ul_confdata_t *tconf;
	sprintf(fnum, "%s.n", fname);
	tconf = ul_initconf(1024);
	if (ul_readconf(work_path, fnum, tconf) == -1) {
		ul_freeconf(tconf);
		return 0;
	}
	ret = ul_getconfint64(tconf, vname, pvalue);
	ul_freeconf(tconf);
	return ret;
}


int ul_readnum_oneint(const char *work_path, const char *fname, char *vname, int *pvalue)
{
	int ret;
	char fnum[PATH_SIZE];
	ul_confdata_t *tconf;

	sprintf(fnum, "%s.n", fname);

	tconf = ul_initconf(1024);

	if (ul_readconf(work_path, fnum, tconf) == -1) {
		ul_freeconf(tconf);
		return 0;
	}

	ret = ul_getconfint(tconf, vname, pvalue);
	ul_freeconf(tconf);

	return ret;
}

int ul_readnum_oneuint64(const char *work_path, const char *fname, char *vname, unsigned long long *pvalue)
{
	int ret;
	char fnum[PATH_SIZE];
	ul_confdata_t *tconf;
	sprintf(fnum, "%s.n", fname);
	tconf = ul_initconf(1024);
	if (ul_readconf(work_path, fnum, tconf) == -1) {
		ul_freeconf(tconf);
		return 0;
	}
	ret = ul_getconfuint64(tconf, vname, pvalue);
	ul_freeconf(tconf);
	return ret;
}

int ul_writenum_init(const char *work_path, const char *fname) {
    FILE *pf;
    char totalname[LINE_SIZE];

    snprintf(totalname, sizeof(totalname), "%s/%s.n", work_path, fname);
    pf = ul_fopen(totalname, "w");
    if (pf == NULL) {
        return -1;
    }
    ul_fclose(pf);
    return 1;
}

static int writenum_one(const char *work_path, const char *fname, const char *vname, void *value, int type)
{
    FILE *pf = NULL;
    char tstra[WORD_SIZE], tstrb[WORD_SIZE];
    char totalname[LINE_SIZE];

    if (work_path == NULL || fname == NULL || vname == NULL || value == NULL) {
        return -3;
    }
    /* open num file */
    snprintf(totalname, sizeof(totalname), "%s/%s.n", work_path, fname);
    pf = ul_fopen(totalname, "a");
    if (pf == NULL) {
        return -1;
    }

    /* write */
    if (sscanf(vname, "%s %s", tstra, tstrb) != 1) {
        slog_write(LL_FATAL, "[writeconf] var's name (%s) error", vname);
        return -2;
    }
    switch (type) {
        case ULTYPE_INT:
            fprintf(pf, "%s : %d\n", vname, *((int *) value));
            break;
        case ULTYPE_UINT:
            fprintf(pf, "%s : %u\n", vname, *((u_int *) value));
            break;
        case ULTYPE_INT64:
            fprintf(pf, "%s : %lld\n", vname, *((long long *) value));
            break;
        case ULTYPE_UINT64:
            fprintf(pf, "%s : %llu\n", vname, *((unsigned long long *) value));
            break;
        case ULTYPE_FLOAT:
            fprintf(pf, "%s : %f\n", vname, *((float *) value));
            break;
        default:
            break;
    }

    /* close file */
    ul_fclose(pf);
    return 1;
}

int ul_writenum_oneint(const char *work_path, const char *fname, const char *vname, int value)
{
    return writenum_one(work_path, fname, vname, &value, ULTYPE_INT);
}

int ul_writenum_onefloat(const char *work_path, const char *fname, const char *vname, float value)
{
    return writenum_one(work_path, fname, vname, &value, ULTYPE_FLOAT);
}


int ul_kreadnum_oneuint(const char *work_path, const char *fname, char *vname, u_int *pvalue)
{
    return kreadnum_one(work_path, fname, vname, pvalue, ULTYPE_UINT);
}

int ul_writenum_oneuint(const char *work_path, const char *fname, const char *vname, u_int value)
{
    return writenum_one(work_path, fname, vname, &value, ULTYPE_UINT);
}

int ul_kreadnum_oneint64(const char *work_path, const char *fname, char *vname, long long *pvalue)
{
    return kreadnum_one(work_path, fname, vname, pvalue, ULTYPE_INT64);
}

int ul_writenum_oneint64(const char *work_path, const char *fname, const char *vname, long long value)
{
    return writenum_one(work_path, fname, vname, &value, ULTYPE_INT64);
}

int ul_kreadnum_oneuint64(const char *work_path, const char *fname, char *vname,
                      unsigned long long *pvalue)
{
    return kreadnum_one(work_path, fname, vname, pvalue, ULTYPE_UINT64);
}

int ul_writenum_oneuint64(const char *work_path, const char *fname, const char *vname,
                      unsigned long long value)
{
    return writenum_one(work_path, fname, vname, &value, ULTYPE_UINT64);
}

