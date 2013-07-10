// -*- encoding = utf-8 -*-
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <signal.h>

#include "tutil.h"
#include "perf_mon.h"

#define POINT_NAME_SIZE	512

typedef struct _mon_time_s {
	struct _mon_time_s * 	p_next;
	struct timeval 			tb;
	struct timeval 			te;
	int			   		 	status;	
}mon_time_t, *mon_time_p;

typedef struct _mon_point_s {
	int 		id;
	char 		name[POINT_NAME_SIZE];	
	int			size;
	mon_time_p 	p_head;
	mon_time_p 	p_cur;
}mon_point_t, *mon_point_p;

typedef struct _perf_mon_s {
	int			max;
	int			num;
	int			pid;
	mon_point_p mon_points;
}perf_mon_t, *perf_mon_p;

#define DFT_PRESET_TIMES 50000

#define	DFT_MAX_POINT	64

static perf_mon_t g_mon;
void perf_statis(int sig_no);

mon_time_p linklist_create(int elem_num, int elem_size)
{
	int i;
	mon_time_p p_head = NULL, p1 = NULL;

	for (i = 0; i < elem_num; i++) {
		MEM_ALLOC(p1, mon_time_p, elem_size, NULL);
		if (!p_head) p_head = p1;
		else {
			p1->p_next = p_head->p_next;
			p_head->p_next = p1;
		}
	}

	return p_head;
}

int perf_mon_init(int max_point, int preset_times)
{
	int i;
	mon_point_p p_temp;

	signal(SIGUSR1, perf_statis);

	memset(&g_mon, 0x00, sizeof(perf_mon_t));

	g_mon.pid = getpid();
	g_mon.max = max_point;
	if (preset_times < 0) g_mon.max = DFT_MAX_POINT;

	MEM_ALLOC(g_mon.mon_points, mon_point_p, sizeof(mon_point_t)*g_mon.max, -1);
	for (i = 0; i < g_mon.max; i++) {
		p_temp = &g_mon.mon_points[i];
		p_temp->size = preset_times;
		if (preset_times < 0) p_temp->size = DFT_PRESET_TIMES;
		p_temp->p_head = linklist_create(p_temp->size, sizeof(mon_time_t));
		p_temp->p_cur = p_temp->p_head;
	}

	return 0;
}

int perf_mon_add(int id, char *name)
{
	int i = g_mon.num;

	if (i >= g_mon.max) {
		// enlarge
		return 1;
	}

	g_mon.mon_points[i].id = id;
	strncpy(g_mon.mon_points[i].name, name, POINT_NAME_SIZE);

	g_mon.num++;

	return 0;
}

int perf_mon_here(int id, int status)
{
	// here, we assume id is the index of array for efficiency.
	mon_point_p p_point = &g_mon.mon_points[id];
	mon_time_p p_cur = p_point->p_cur;

	switch (status) {
		case ST_START: // start
			gettimeofday(&p_cur->tb, NULL);
			p_cur->status |= ST_START;
			break;

		case ST_END:
			gettimeofday(&p_cur->te, NULL);
			p_cur->status |= ST_END;
			if (!p_cur->p_next) {
				p_cur->p_next = linklist_create(p_point->size/2 + 1, sizeof(mon_time_t));
				p_point->size += p_point->size/2 + 1;
			}
			p_point->p_cur = p_cur->p_next;
			break;

		default:
			return -1;
	}

	return 0;
}

void perf_statis(int sig_no)
{
	int fd, i, len;
	int fd2;
	char file[512];
	char path[512];
	char buf[512];
	time_t now;
	struct tm local = { 0 };
	mon_point_p p_point = NULL;
	mon_time_p p_cur = NULL;

	time(&now);
	localtime_r(&now, &local);
	fprintf(stdout, "%s sig_no =%d\n", __FUNCTION__, sig_no);
	snprintf(path, sizeof(path), "%d-%d-%d_%d.%d.%d", 2000 + local.tm_year-100, local.tm_mon+1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
	mkdir(path, 0777);
	snprintf(file, sizeof(file), "%s/file_list", path);

	fd2 = open(file, O_WRONLY|O_CREAT, S_IRWXU);

	for (i = 0; i < g_mon.num; i++) {
		p_point = &g_mon.mon_points[i];
		p_cur = p_point->p_head;
		len = snprintf(file, sizeof(file), "%s/%d.%d.%s.perf", path, g_mon.pid, p_point->id, p_point->name);
		write(fd2, file, len);
		write(fd2, "\n", 1);

		fd = open(file, O_WRONLY|O_CREAT, S_IRWXU);
		//fprintf(stdout, "---%p\n", p_cur);
		while (p_cur) {
			//fprintf(stdout, "--status=%d\n", p_cur->status);
			if (p_cur->status == 0x03) {// start|end
				len = snprintf(buf, sizeof(buf), "%d|%d|%s|%ld.%ld|%ld.%ld\n", g_mon.pid, p_point->id, p_point->name, p_cur->tb.tv_sec, p_cur->tb.tv_usec, p_cur->te.tv_sec, p_cur->te.tv_usec);
				write(fd, buf, len);
			} else {
				//fprintf(stdout, "----%d\n", p_cur->status);
			}
			p_cur->status = 0;
			if (p_point->p_cur == p_cur->p_next) break;
			p_cur = p_cur->p_next;
		}
		//fprintf(stdout, "--------\n");
		fsync(fd);

		close(fd);
		p_point->p_cur = p_point->p_head;
	}
	fsync(fd2);
	close(fd2);
}

int perf_mon_destroy()
{
	return 0;
}

