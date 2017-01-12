#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "flp.h"
#include "temperature.h"

enum Pos {Leaf, Horizontal, Vertical};

struct fp_st_lh
{
	int block_id;
	float w;
	float h;
	float left;
	float bottom;
	enum Pos pos;
	struct fp_st_lh *LChild;
	struct fp_st_lh *RChild;
};

void floor_plan_run();

