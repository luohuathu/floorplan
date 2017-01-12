#include "util_lh.h"

const float run_time = 1*60*CLOCKS_PER_SEC;
const float RateThreshold = 0.00005;
float step_size = 0.0000001;
char *input_file = "input.txt";
char *flp_file = "floorplan.txt";
char *power_file = "power.txt";
char *output_file = "114248756.txt";


//This function reads the given input .txt file and generates the floorplan data structure and power file for later use
float read_input_file(int *N, float* overhead, char* input_file, char * power_file, struct fp_st_lh **fp_st)
{
	FILE *fin, *fpwr;
	fin = fopen(input_file,"r");
	fpwr = fopen(power_file,"w");
	
	fscanf(fin, "%d", N);
	fscanf(fin, "%f", overhead);
	
	float w, h, pwr;
	float Area = 0;
	int i;
	*fp_st = malloc(*N *sizeof(struct fp_st_lh));
	for(i = 0; i < *N; i++)
	{
		fscanf(fin, "%f %f %f", &w, &h, &pwr);
		fprintf(fpwr, "block%d	%f\n", i, pwr);
		Area += w*h;	
		struct fp_st_lh temp_struct = {i, w, h, 0, 0, Leaf, NULL, NULL};
		(*fp_st)[i] = temp_struct;
	}

        fclose(fin);
	fclose(fpwr);
	return Area;
}

//This function writes the floorplan data structure to floorplan file for temperature calculation
void write_to_flpfile(int N, struct fp_st_lh *fp_st, char *flp_file)
{
	FILE *flp;
	flp = fopen(flp_file,"w");
	int i;
	for(i = 0; i < N; i++)
	{
		fprintf(flp, "block%d	%f	%f	%f	%f\n",fp_st[i].block_id, fp_st[i].w, fp_st[i].h, fp_st[i].left, fp_st[i].bottom );
	}
	fclose(flp);	
}

//write floor plan information to final output file
void write_to_output(int N, struct fp_st_lh *fp_st, char *output_file)
{
	FILE *flp;
	flp = fopen(output_file,"w");
	int i;
	for(i = 0; i < N; i++)
	{
		fprintf(flp, "%f	%f\n", fp_st[i].left, fp_st[i].bottom );

	}
	fclose(flp);	
}


//update the width and height of a block according to its children and their relative position
void update_w_h(struct fp_st_lh *root, float epsilon)
{
	if(!root || root->pos == Leaf) return;
	if(!root->LChild && root->RChild)
	{
		root->w = root->RChild->w;
		root->h = root->RChild->h;
	}
	else if(root->LChild && !root->RChild)
	{
		root->w = root->LChild->w;
		root->h = root->LChild->h;
	}
	else if(root->pos == Vertical)
	{
		root->w = root->LChild->w > root->RChild->w ? root->LChild->w: root->RChild->w;
		root->h = root->LChild->h + root->RChild->h + epsilon;
	}
	else 
	{
		root->w = root->LChild->w + root->RChild->w + epsilon;
		root->h = root->LChild->h > root->RChild->h ? root->LChild->h: root->RChild->h;
	}
}

struct fp_st_lh* copy_array(int N, struct fp_st_lh *myArray)
{
	int i;
	struct fp_st_lh *temp = malloc(sizeof(struct fp_st_lh) * N);
	for(i = 0; i < N; i++)
		temp[i] = myArray[i];
	return temp;
}

int * sort_array(int N, struct fp_st_lh *myArray)//bubble sort myArray according to block area
{ 
	int * index_array = malloc(sizeof(int) * N);
	struct fp_st_lh *newArray = copy_array(N, myArray);
	int j = 0;
	int i = 0;
	int swapped = 1;
	struct fp_st_lh temp;
	while(swapped)
	{
		swapped = 0;
		j++;
		for (i = 0; i < N - j; i++)
		{
			if(newArray[i].w * newArray[i].h > newArray[i+1].w * newArray[i+1].h)
			{
				temp = newArray[i];
				newArray[i] = newArray[i+1];
				newArray[i+1] = temp;
				swapped = 1;	
			}
		}
	}
	for(i = 0; i < N; i++)
		index_array[i] = newArray[i].block_id;
	free(newArray);
	return index_array;
}


//generate an initial solution
struct fp_st_lh * init_solution(double min_l, int start_id, int end_id, int N, struct fp_st_lh *myArray, int * index_array, float epsilon)
{
	if(!myArray) return NULL;
	if(start_id == end_id)
	{
		struct fp_st_lh * temp = malloc(sizeof(struct fp_st_lh));
		*temp = myArray[index_array[start_id]];
		return temp;
	}
	
	int middle = (start_id + end_id)/2;
	struct fp_st_lh * LeftHalf = init_solution(min_l, start_id, middle, N, myArray, index_array, epsilon);
	struct fp_st_lh * RightHalf = init_solution(min_l, middle+1, end_id, N, myArray, index_array, epsilon);
	if(!LeftHalf) return RightHalf;
	if(!RightHalf) return LeftHalf;
	
	struct fp_st_lh *parent = malloc(sizeof(struct fp_st_lh));
	parent->LChild = LeftHalf;
	parent->RChild = RightHalf;
	float w_v = parent->LChild->w > parent->RChild->w ? parent->LChild->w : parent->RChild->w;
	float h_v = parent->LChild->h + parent->RChild->h + epsilon;
	float w_h = parent->LChild->w + parent->RChild->w + epsilon;
	float h_h = parent->LChild->h > parent->RChild->h ? parent->LChild->h : parent->RChild->h;
	if(w_v * h_v < w_h * h_h || (w_v * h_v == w_h * h_h && w_v + h_v < w_h * h_h) || w_v > min_l) 
		parent->pos = Vertical;
	else parent->pos = Horizontal;
	update_w_h(parent, epsilon);	
	parent->block_id = N;
	parent->left = 0;
	parent->bottom = 0;
	return parent;
}

//read the current tree to get the corresponding floor plan
void read_tree(float Block_left, float Block_bottom, struct fp_st_lh* root, struct fp_st_lh **myArray)
{
	if(!root) return;
	else if(root->pos == Leaf)  //Leaf node, read to array
	{
		(*myArray)[root->block_id].left = Block_left;
		(*myArray)[root->block_id].bottom = Block_bottom;
		return;
	}
	else if(!root->LChild && root->RChild)     
		read_tree(Block_left, Block_bottom, root->RChild, myArray);
	else if(root->LChild && !root->RChild)
		read_tree(Block_left, Block_bottom, root->LChild, myArray);

	else if(root->pos == Vertical)
	{
		read_tree(Block_left, Block_bottom, root->LChild, myArray);
		read_tree(Block_left, Block_bottom + root->h - root->RChild->h, root->RChild, myArray);
	}
	else if(root->pos == Horizontal)
	{
		read_tree(Block_left, Block_bottom, root->LChild, myArray);
		read_tree(Block_left + root->w - root->RChild->w, Block_bottom, root->RChild, myArray);
	}
}

void move_0(struct fp_st_lh *root, float epsilon)//move type0: switch two adjacent operands
{
	if(!root || root->pos == Leaf) return;
	
	move_0(root->LChild, epsilon);
	move_0(root->RChild, epsilon);
	update_w_h(root, epsilon);
	int coin = rand()%100 > 80;
	if(!coin) return;
	struct fp_st_lh *temp;
	temp = root->LChild;
	root->LChild = root->RChild;
	root->RChild = temp;
}

void move_1(struct fp_st_lh *root, float epsilon)//move type1: complement some chain. i.e: change vertical positions to horizontal, vice versa.
{
	if(!root || root->pos == Leaf) return;
	
	move_1(root->LChild, epsilon);
	move_1(root->RChild, epsilon);
	int coin = rand()%100 > 80;
	if(coin) root->pos = (root->pos == Vertical)? Horizontal: Vertical;
	update_w_h(root, epsilon);
}

void move_2(struct fp_st_lh *root, float epsilon)//move type2: break the connection between two adjacent blocks by changing the parent-children relationship
{
	if(!root || root->pos == Leaf) return;
	move_2(root->RChild, epsilon);
	move_2(root->LChild, epsilon);
	
	int coin = rand()%100 > 80;
	if(coin && root->LChild && root->LChild->pos != Leaf)
	{
		struct fp_st_lh *temp;
		temp = root->RChild;
		root->RChild = root->LChild->RChild;
		root->LChild->RChild = temp;  
		update_w_h(root->LChild, epsilon);	
	}
	update_w_h(root, epsilon);
}

void insert_space(struct fp_st_lh *root, float epsilon, int k)
{
	if(!root || root->pos == Leaf) return;
	insert_space(root->LChild, epsilon, k);
	insert_space(root->RChild, epsilon, k);
	update_w_h(root, epsilon);
	float size = (rand()% k + 1)*epsilon;
	update_w_h(root, size);
}

struct fp_st_lh* copy_tree(struct fp_st_lh *root) //deep copy
{
	if(!root) return NULL;

	struct fp_st_lh *newTree = malloc(sizeof(struct fp_st_lh));
	newTree->block_id = root->block_id;
	newTree->w = root->w;
	newTree->h = root->h;
	newTree->left = root->left;
	newTree->bottom = root->bottom;
	newTree->pos = root->pos;
	newTree->LChild = copy_tree(root->LChild);
	newTree->RChild = copy_tree(root->RChild);
	return newTree;
}

void free_st_tree(struct fp_st_lh *root)
{
	if(!root) return;
	free_st_tree(root->LChild);
	free_st_tree(root->RChild);
	free(root);
}

void check_overlap(int N, struct fp_st_lh * blocks)
{
	int i, j;
	for(i = 0; i < N; i ++)
	  for(j = i+1; j < N; j++)
		{
			char f1 = blocks[i].left + blocks[i].w <= blocks[j].left;
   			char f2 = blocks[i].bottom + blocks[i].h <= blocks[j].bottom;
  			char f3 = blocks[j].left + blocks[j].w <= blocks[i].left;
   			char f4 = blocks[j].bottom + blocks[j].h <= blocks[i].bottom;

  			 if(!(f1 || f2 || f3 || f4))
   			{
  				 printf("block%d and block%d overlap\n", i, j);
    				 printf("i_w = %.8f i_h = %.8f i_left = %.8f i_bottom =  %.8f\nj_w = %.8f j_h = %.8f j_left = %.8f j_bottom =  %.8f \n", blocks[i].w, blocks[i].h, blocks[i].left, blocks[i].bottom, blocks[j].w, blocks[j].h, blocks[j].left, blocks[j].bottom );
			}
		}
}

void floor_plan_run() 
{
	int N, coin_bit, k;
	int *Index;
	clock_t begin_time = clock();
	float Area, Area_Overhead;
	float Accepts = 1;
	float Total = 1;
	float AcceptRate = 1;
	float epsilon = step_size;

	struct fp_st_lh *fp_st, *root;
	Area = read_input_file(&N, &Area_Overhead, input_file, power_file, &fp_st);
	printf("N = %d, Area = %f Area_Overhead = %f\n", N, Area, Area_Overhead);
	
	thermal_config_t config = default_thermal_config();
	double min_l = config.s_sink > config.s_spreader? config.s_spreader:config.s_sink;
	Index = sort_array(N, fp_st);	
	root = init_solution(min_l, 0, N-1, N, fp_st, Index, epsilon);

	flp_t *flp;
	double MaxTemp_Local = 600;
	double MaxTemp_Global = 600;
	double NewTemp;

	// read_temp(model, temp, init_temp_file, FALSE);
	// transient solver    
	//compute_temp(model, power, temp, delta_t);
 
	clock_t current_time = clock();
	while(AcceptRate > RateThreshold && (float)(current_time - begin_time) < run_time)   //simulated annealing
	{
		current_time = clock();
		struct fp_st_lh *tree_b;
		tree_b = copy_tree(root); 
		coin_bit = rand()%3;
		switch(coin_bit)
		{
			case 0: move_0(tree_b, epsilon); break;
			case 1: move_1(tree_b, epsilon); break;
			case 2: move_2(tree_b, epsilon); break;
		}
		if(tree_b->w > min_l || tree_b->h > min_l || tree_b->w * tree_b->h > Area*(1 + Area_Overhead))
		{ 
			free_st_tree(tree_b); 
			continue;
		}
		for(k = 2; k < 100; k++)
		{
			if(tree_b->w > min_l || tree_b->h > min_l || tree_b->w * tree_b->h > Area*(1 + Area_Overhead))
			{
				insert_space(tree_b, epsilon, k);
				continue;
			}
			read_tree(0, 0, tree_b, &fp_st);
			write_to_flpfile(N, fp_st, flp_file);
			flp = read_flp(flp_file, FALSE);
			RC_model_t *model = alloc_RC_model(&config, flp);
			populate_R_model(model, flp);
			populate_C_model(model, flp); 

			double *power, *temp;
			temp = hotspot_vector(model);
			power = hotspot_vector(model);
			read_power(model, power, power_file);

			steady_state_temp(model, power, temp);
			NewTemp = find_max_temp(model, temp);
		
			if(NewTemp < MaxTemp_Local || rand() % (int)(exp(2200.0/MaxTemp_Local)) == 1) //accept the change
			{
				free_st_tree(root);
				root = copy_tree(tree_b);
				MaxTemp_Local = NewTemp;
				Accepts++;
			}
			if(NewTemp < MaxTemp_Global)
			{
				MaxTemp_Global = NewTemp;
				read_tree(0, 0, root, &fp_st);
				check_overlap(N, fp_st);
				write_to_output(N, fp_st,output_file);
			}
			Total++;
			AcceptRate = Accepts/Total;
			free_dvector(temp);
			free_dvector(power);
			delete_RC_model(model);
			free_flp(flp, FALSE);
			insert_space(tree_b, epsilon, k);
		//printf("Total feasible solutions = %f\n", Total);
		//printf("time: %f\n", (float)(current_time - begin_time));
		//printf("Local optimal solution: temperature = %f\n", MaxTemp_Local);
		}
		free_st_tree(tree_b);	
	}
	printf("Global optimal solution: temperature = %f\n", MaxTemp_Global);
	free(fp_st);
	free_st_tree(root);
	return;
}

