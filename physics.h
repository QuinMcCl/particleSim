#ifndef __physics_H
#define __physics_H

#include <stdlib.h>

#include "particle.h"
#include "BoundBox.h"

void physics_step(Particle * particle_list, int numParticles, int * sub_particles, float timestep);


#endif
