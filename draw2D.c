#include "draw2D.h"

void draw_image(unsigned int * PixelBuffer, int width, int height, Particle * particle_list, int numParticles, int * sub_particles)
{
    Box * Boxes = build_boxes(particle_list, numParticles, sub_particles,0,0.0);
    #pragma omp parallel for
    for(int i = 0; i < width * height; i++) {
		
		int x = i % width;
		int y = i / height;
        
        vec_t point = (vec_t){(float)x,(float)y};
        int particle_count = 0;
        int *  particle_indexes = get_within_bounds(Boxes, 0, point, point, &particle_count);
        float color_val = 0.0f;
        for(int j = 0; j < particle_count; j++)
        {
            int index = particle_indexes[j];
            float distSq = 0.0f;
            for(int d = 0; d < dims; d++)
            {
                distSq += pow((particle_list[index].pos.v[d] - point.v[d])/particle_list[index].radius,2);
            }
            if(distSq < 1)
            {
                color_val += sqrt(1 - distSq);
            }
        }
        color_val = fmod(color_val,1.0);
        free(particle_indexes);
        color_t color = (color_t){1.0f,color_val,color_val,color_val};
		PixelBuffer[x + y * width] = color_to_int(color);

	}
    free(Boxes);
}
