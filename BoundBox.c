#include "BoundBox.h"


//returns the list of indexes of every particle index that intersects with bounds
int * get_within_bounds(Box * Boxes, int box_num, vec_t min, vec_t max, int * P_num)
{
    int * ret = (int *) malloc(sizeof(int));
    if (Boxes == NULL)return NULL;
    int within = 1;
    for(int i = 0; i < dims; i++)
    {
        within &= ((Boxes[box_num].max_pos.v[i] >= min.v[i]) && (Boxes[box_num].min_pos.v[i] <= max.v[i]));
    }
    
    if (within)
    {
        if(Boxes[box_num].P_index < 0)
        {
            for(int i = 0; i < (1<<dims); i++)
            {
                if(Boxes[box_num].children[i] > 0)
                {
                    int childnum = 0;
                    int * child_ret = get_within_bounds(Boxes, Boxes[box_num].children[i], min, max, &childnum);
                    
                    ret = (int *)realloc(ret, sizeof(int) * (*P_num + childnum));
                    memcpy(ret + (*P_num), child_ret, sizeof(int)*childnum);
                    free(child_ret);
                    
                    *P_num += childnum;
                }
            }
        }
        else
        {
            ret[0] = Boxes[box_num].P_index;
            *P_num += 1;
        }
    
    }
    
    
    return ret;
}


Box * build_boxes(Particle * particle_list, int numParticles, int sub_particles[numParticles],int BoxOffset, float timestep)
{
    //printf("Building Box %d, Num Particles %d\n",BoxOffset,numParticles);
	Box * parent = (Box *) malloc(sizeof(Box));
	
    parent->numBoxes = 1;
    parent->P_index = -1;
    parent->num_P = 0;
    
	for(int i = 0; i < dims; i++)
	{
		parent->max_pos.v[i] = NAN;
		parent->min_pos.v[i] = NAN;
		parent->mid_pos.v[i] = 0.0;
	}
	
	if(numParticles <1)
	{
		free(parent);
		return NULL;
		
	}
	else if(numParticles == 1)
	{
        //printf("Leaf Box\n");
		parent->num_P = numParticles;
		parent->P_index = sub_particles[0];
		
		for(int j = 0; j < dims; j++)
		{
			parent->max_pos.v[j] = fmax(particle_list[parent->P_index].pos.v[j], particle_list[parent->P_index].pos.v[j] + (particle_list[parent->P_index].vel.v[j]*timestep)) + particle_list[parent->P_index].radius;
			parent->min_pos.v[j] = fmin(particle_list[parent->P_index].pos.v[j], particle_list[parent->P_index].pos.v[j] + (particle_list[parent->P_index].vel.v[j]*timestep)) - particle_list[parent->P_index].radius;
		}
		return parent;
	}
	//printf("Parent Box\n");
    //for(int j = 0; j < numParticles; j++)printf("SubParticle %d\n",sub_particles[j]);
	int  * sub_particle_list[(1<<dims)];
	int sub_particle_count[(1<<dims)];
    
	for(int i = 0; i < (1<<dims); i++)
	{
		sub_particle_list[i] = (int *) malloc(sizeof(int));
		sub_particle_count[i] = 0;
	}
    
	for(int i = 0; i < numParticles; i++)
	{
		int index = sub_particles[i];
		for(int j = 0; j < dims; j++)
		{
            //printf("adding particle %d, pos %g to mid pos\n",index,(particle_list[index].pos.v[j]));
			parent->mid_pos.v[j] = (particle_list[index].pos.v[j]/ ((float)numParticles)) + parent->mid_pos.v[j];
		}
	}
    //for(int j = 0; j < dims; j++)printf("ParentMid D %d:%g\n",j, parent->mid_pos.v[j]);
	for(int i = 0; i < numParticles; i++)
	{
		int region = 0;
		int index = sub_particles[i];
		for(int j = 0; j < dims; j++)
		{
			if (particle_list[index].pos.v[j] >= parent->mid_pos.v[j])region +=(1<<j);
		}
        //printf("Particle %d, reg %d\n",index, region);
		sub_particle_count[region]+=1;
		sub_particle_list[region] = realloc(sub_particle_list[region], sub_particle_count[region] * sizeof(int));
		sub_particle_list[region][sub_particle_count[region] - 1] = index;
	}
	
	for(int i = 0; i < (1<<dims); i++)
	{
		Box * child = build_boxes(particle_list, sub_particle_count[i], sub_particle_list[i],parent->numBoxes + BoxOffset,timestep);
        free(sub_particle_list[i]);
		if (child != NULL)
		{
            
			parent = (Box *) realloc(parent, sizeof(Box) * (parent->numBoxes + child->numBoxes));
            memcpy(parent + parent->numBoxes, child, (sizeof(Box)*child->numBoxes));
            
			for(int j = 0; j < dims; j++)
			{
				parent->min_pos.v[j] = fmin(parent->min_pos.v[j],child->min_pos.v[j]);
				parent->max_pos.v[j] = fmax(parent->max_pos.v[j],child->max_pos.v[j]);
			}
			parent->children[i] = parent->numBoxes + BoxOffset;
			parent->numBoxes += child->numBoxes;
            parent->num_P += child->num_P;
			free(child);
		}
		else
		{
			parent->children[i] = -1;
		}
	}
    //printf("Done Building Box %d\n",BoxOffset);
	return parent;
}
