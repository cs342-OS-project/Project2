#ifndef SCHEDULING
#define SCHEDULING

// DECLARATIONS

#define MIN_PRIO -20
#define MAX_PRIO 19

static const int prio_to_weight[40] = {
    88761,      71755,      56484,      46273,      36291,
    29154,      23254,      18705,      14949,      11916,
    9548,       7620,       6100,       4904,       3906,
    3121,       2501,       1991,       1586,       1277,
    1024,       820,        655,        526,        423,
    335,        272,        215,        172,        137,
    110,        87,         70,         56,         45,
    36,         29,         23,         18,         15 
};

static const int min_granularity = 10;
static const int sched_latency = 100;

int map_to_weight(int prio);

// IMPLEMENTATION

int map_to_weight(int prio)
{
    return prio_to_weight[prio + MIN_PRIO * (-1)];
}

#endif