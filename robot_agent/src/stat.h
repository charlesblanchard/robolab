typedef struct s_stat_t
{
	int nb_overruns;
	int task_nb_overruns[8];

	int nb_victim_found;
	double total_distance;

} stat_t;



void init_stat(stat_t *s);

void stat_overruns_add(stat_t *s, int task_id);

void stat_victim_precision(stat_t *s, victim_t found);

double stat_victim_average(stat_t *s);