#ifndef __STAT_H
#define __STAT_H


typedef struct s_stat_t
{
	int nb_overruns;
	int task_nb_overruns[8];
	
	struct timeval victim_event;
	struct timeval stop_event;

	int nb_victim_found;
	double total_distance;

	int go_ahead_reception;
	int go_ahead_flag;
	int executed_major_cycle;
	
	int nb_msg_sent[4];

} stat_t;



void init_stat(stat_t *s);

void stat_overruns_add(stat_t *s, int task_id);

void stat_go_ahead_reception_add(stat_t *s);
void stat_executed_major_cycle_add(stat_t *s);
double stat_get_go_ahead_miss(stat_t s);

void stat_victim_precision(stat_t *s, victim_t found);
double stat_victim_average(stat_t *s);

void print_stat(stat_t *s);


#endif
