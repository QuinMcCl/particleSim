#include "physics.h"
#include <math.h>
//should move by one physics step

float vec_dot(vec_t a, vec_t b)
{
    float product_sum = 0.0f;
    for(int d = 0; d < dims; d++)
    {
        product_sum += a.v[d] * b.v[d];
    }
    return product_sum;
}
vec_t vec_scale(vec_t a, float b)
{
    vec_t product;
    for(int d = 0; d < dims; d++)
    {
        product.v[d] = a.v[d] * b;
    }
    return product;
}
vec_t vec_sub(vec_t a, vec_t b)
{
    vec_t diff;
    for(int d = 0; d < dims; d++)
    {
        diff.v[d] = a.v[d] - b.v[d];
    }
    return diff;
}
vec_t vec_add(vec_t a, vec_t b)
{
    vec_t sum;
    for(int d = 0; d < dims; d++)
    {
        sum.v[d] = a.v[d] + b.v[d];
    }
    return sum;
}

void physics_step(Particle * particle_list, int numParticles, int * sub_particles, float timestep)
{
    while(timestep > 0)
    {   
        Box * Boxes = build_boxes(particle_list, numParticles, sub_particles,0,timestep);
        //TODO parallel find collision time step
        float collision_step = timestep;
        
        int index[2];
        #pragma omp parallel for
        for (int i = 0; i < numParticles; i++) {
            //printf("Particle %d\n",i);
            int pnum = 0;
            vec_t min;
            vec_t max;
            for(int j = 0; j < dims; j++)
		    {
			    max.v[j] = fmax(particle_list[i].pos.v[j], particle_list[i].pos.v[j] + (particle_list[i].vel.v[j]*timestep)) + particle_list[i].radius;
			    min.v[j] = fmin(particle_list[i].pos.v[j], particle_list[i].pos.v[j] + (particle_list[i].vel.v[j]*timestep)) - particle_list[i].radius;
		    }
            int* colls = get_within_bounds(Boxes, 0, min, max, &pnum);
            if (colls != NULL) {
                //printf("NumColls %d\n",pnum);
                for (int j = 0; j < pnum; j++) {
                    if (colls[j] >= i) continue;
                    //printf("Collides %d\n",colls[j]);
                    vec_t posdiff = vec_sub(particle_list[i].pos, particle_list[colls[j]].pos); // vector difference in particle positions
                    vec_t veldiff = vec_sub(particle_list[i].vel, particle_list[colls[j]].vel); // vector difference in particle velocities
                    
                    float pos_dot = vec_dot(posdiff,posdiff);
                    float pos_vel_dot = vec_dot(posdiff,veldiff);
                    float vel_dot = vec_dot(veldiff,veldiff);
                    if (vel_dot == 0)continue;
                    
                    float r0 = particle_list[i].radius;
                    float r1 = particle_list[colls[j]].radius;
                    float maxr = fmax(r0, r1);
                    float squareR = pow((r0 + r1),2);
                    
                    if(pos_dot < pow(maxr,2.0))
                    {
                        squareR = pow((r0 - r1),2);
                        
                    }
                    
                    float t_part = pow((pos_vel_dot),2.0) - (vel_dot * (pos_dot - squareR));
                    
                    if (t_part < 0)continue;
                    float t0 = (sqrt(t_part)-pos_vel_dot)/vel_dot;
                    float t1 = ((-1 * sqrt(t_part))-pos_vel_dot)/vel_dot;
                    double t;
                    if(t0 < 0 && t1 < 0)
                    {
                        continue;
                    }
                    else if (t0 < 0)
                    {
                        t = t1;
                    }
                    else if (t1 < 0)
                    {
                        t = t0;
                    }
                    else
                    {
                        t = fmin(t0,t1);
                    }
                    if (t < 0)continue;
                    if (t < collision_step) {
                        #pragma omp critical
                        {
                            index[0] = i;
                            index[1] = colls[j];
                            collision_step = t;
                        }
                    }
                    
                }
                
                free(colls);
            }
            
        }

        //printf("Done Collisions, CSTEP %g\n",collision_step);
        //step collision timestep
        //TODO parallel step collision time step
        for(int i = 0; i < numParticles; i++)
        {
            for(int j = 0; j < dims; j++)
            {
                particle_list[i].pos.v[j] += particle_list[i].vel.v[j]* collision_step;
            }
        }
        
        //printf("Done Move\n");
        //TODO actually resolve collisions
        //resolve collisions
        if (collision_step < timestep) {
            //printf("Bounce %d %d\n",index[0],index[1]);
            float m0 = particle_list[index[0]].mass;
            float m1 = particle_list[index[1]].mass;
            vec_t v0 = particle_list[index[0]].vel;
            vec_t v1 = particle_list[index[1]].vel;
           
            vec_t dp01 = vec_sub(particle_list[index[0]].pos, particle_list[index[1]].pos);
            vec_t dp10 = vec_sub(particle_list[index[1]].pos, particle_list[index[0]].pos);
           
            vec_t v01i = vec_scale(dp01 , vec_dot(v0 , dp01) / vec_dot(dp01 , dp01));
            vec_t v10i = vec_scale(dp10 , vec_dot(v1 , dp10) / vec_dot(dp10 , dp10));
           
            vec_t vu0 = vec_sub(v0, v01i);
            vec_t vu1 = vec_sub(v1, v10i);
           
            vec_t v01f;
            vec_t v10f;
            
            if(index[0] == 0)
            {
                v01f = vec_scale(vu0,0.0);
                v10f = vec_scale(v10i,-1.0);
            }
            if(index[1] == 0)
            {
                v01f = vec_scale(v01i,-1.0);
                v10f = vec_scale(vu1,0.0);
            }
            else
            {
                v01f = vec_add(vec_scale(v01i,((m0 - m1) / (m0 + m1))) , vec_scale(v10i,((2.0 *  m1)/(m0 + m1))));
                v10f = vec_add(vec_scale(v10i,((m1 - m0) / (m1 + m0))) , vec_scale(v01i,((2.0 *  m0)/(m1 + m0))));
            }
            
           
            vec_t v0f = vec_add(v01f, vu0);
            vec_t v1f = vec_add(v10f, vu1);
           
           
           
            for(int j = 0; j < dims; j++)
            {
                particle_list[index[0]].vel.v[j] = v0f.v[j];
                particle_list[index[1]].vel.v[j] = v1f.v[j];
            }
        }
        
        //printf("Done Bounce\n");
        
        timestep -= collision_step;
        free(Boxes);
        
    }
	
    return;
}
