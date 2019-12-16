#include "task.h"

stat_t g_stat;


void init_stat(stat_t *s)
{	
	s->nb_overruns = 0;

	for(int i=0; i<8; i++)
		s->task_nb_overruns[i] = 0;
		
	s->nb_victim_found = 0;
	s->total_distance = 0.0;
	
	s->go_ahead_reception = 0;
	s->go_ahead_flag = 0;
	s->executed_major_cycle = 0;
	
	for(int i=0; i<4; i++)
		s->nb_msg_sent[i] = 0;
}


void stat_overruns_add(stat_t *s, int task_id){
	s->task_nb_overruns[task_id]++;
	s->nb_overruns++;

	printf("Overruns %i\n",task_id);
}

void stat_go_ahead_reception_add(stat_t *s){
	s->go_ahead_flag ++;
}

void stat_executed_major_cycle_add(stat_t *s){
	if(s->go_ahead_flag != 0)
	{
		s->go_ahead_reception ++;
		s->go_ahead_flag = 0;
	}
	s->executed_major_cycle ++;
}

double stat_get_go_ahead_miss(stat_t s)
{
	return ((double)(s.executed_major_cycle - s.go_ahead_reception)/(double)s.executed_major_cycle)*100;
}

static const victim_t THEORICAL[24] = 
{
	{ 340,	340, 	"020058F5BD\0" },
	{ 975,	1115,	"020053A537\0" },
	{ 1845,	925, 	"020053E0BA\0" },
	{ 2670,	355, 	"01004B835E\0" },
	{ 3395,	870, 	"020053C80E\0" },
	{ 4645,	910,	"020058100D\0" },
	{ 4800,	250,	"0200580B96\0" },
	{ 5395,	1060,	"02005345B6\0" },
	{ 5830,	1895, 	"020058F121\0" },
	{ 5110,	2390, 	"0200581B9E\0" },
	{ 5770,	3790, 	"020058066F\0" },
	{ 4500,	3190, 	"020058212D\0" },
	{ 4315,	3200, 	"020058022D\0" },
	{ 4150,	1810, 	"0200581542\0" },
	{ 3720,	3710, 	"0200534E5C\0" },
	{ 2580,	3770, 	"020053AB2C\0" },
	{ 2970,	2805, 	"01004A11E8\0" },
	{ 3030,	2070, 	"020053E282\0" },
	{ 3120,	1965,	"0200553505\0" },
	{ 2880,	1840, 	"01004751A2\0" },
	{ 1890,	2580, 	"02005097C0\0" },
	{ 985,	3020, 	"020053BF78\0" },
	{ 730,	3175, 	"020056D0EF\0" },
	{ 320,	1800, 	"01004BDF7B\0" }
};

void stat_victim_precision(stat_t *s, victim_t found)
{
	int i;

	for(i=0; i<24; i++)
	{
		if (!strcmp(found.id, THEORICAL[i].id))
            break;
	}

	if(i==24)
	{
		//not found chelou
		printf("ERROR: Victim not found in the list %s\n", found.id );
		return;
	}

	s->nb_victim_found++;

	double distance = sqrt(	pow(found.x-THEORICAL[i].x,2) + pow(found.y-THEORICAL[i].y,2) );

	s->total_distance += distance;

	printf("Euclidian distance between found victim and theo position: %f\n",distance);
}

double stat_victim_average(stat_t *s)
{
	return s->total_distance / s->nb_victim_found;
}


void print_stat(stat_t *s){
	
	printf("Nb overruns : %d\nNb victim found : %d\nRatio go ahead missed : %f\n",s->nb_overruns, s->nb_victim_found, stat_get_go_ahead_miss(*s));
	/*
	for(int i=0; i<8; i++){
		printf("%d ",s->task_nb_overruns[i]);
	}
	*/
	
	printf("\nNb msg sent :");
	for(int i=0; i<4; i++){
		printf(" %d",s->nb_msg_sent[i]);
		s->nb_msg_sent[i] = 0;
	}
	printf("\n\n");
}
