#include "particle.h"

Particle * init_particles(int numParticles, vec_t min_pos, vec_t max_pos)
{
    //generates some random particles within the bounds of min and max
	Particle * particle_list = (Particle *) malloc(sizeof(Particle) * numParticles);
    
    float size = 0.0;
    for(int j = 0; j < dims; j++)
    {
        size += pow((max_pos.v[j] - min_pos.v[j])/2.0,2.0);
		particle_list[0].pos.v[j] = (max_pos.v[j] - min_pos.v[j])/2.0;
		particle_list[0].vel.v[j] = 0.0;
	}
	particle_list[0].radius = sqrt(size);
    particle_list[0].mass = INFINITY;
    
    
	for(int i = 1; i < numParticles; i++)
	{
		for(int j = 0; j < dims; j++)
		{
			particle_list[i].pos.v[j] = (float)rand()/(float)(RAND_MAX) * (max_pos.v[j] - min_pos.v[j]) + min_pos.v[j];
			particle_list[i].vel.v[j] = ((float)rand()/(float)(RAND_MAX) * (max_pos.v[j] - min_pos.v[j]) - (max_pos.v[j] - min_pos.v[j])/2.0 ) * 0.01;
		}
		particle_list[i].radius = (float)rand()/(float)(RAND_MAX) * sqrt(size)/(float)numParticles + 10;
        particle_list[i].mass = (float)rand()/(float)(RAND_MAX) * 50.0f + 25.0f;
	}
	return particle_list;
}
