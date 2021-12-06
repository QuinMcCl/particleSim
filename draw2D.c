#include "draw2D.h"

void draw_image(unsigned int * PixelBuffer, int width, int height, Particle * particle_list, int numParticles, int * sub_particles)
{
    Box * Boxes = build_boxes(particle_list, numParticles, sub_particles,0,0.0);
    //printf("DoneBuildBoxes\n");
    #pragma acc parallel loop \
    create(PixelBuffer[:height*width]) \
    copyout(PixelBuffer[:height*width]) \
    copy(particle_list,particle_list[:numParticles],width,height,Boxes,Boxes[:Boxes->numBoxes]) \
    reduction(+:PixelBuffer) \
    independent
    for(int i = 0; i < width * height; i++) {
		//printf("Pix %d\n",i);
		int x = i % width;
		int y = i / height;
        
        vec_t point = (vec_t){(float)x,(float)y};
        int particle_count = 0;
        int *  particle_indexes = get_within_bounds(Boxes, 0, point, point, &particle_count);
        float color_val = 0.0f;
        
        //printf("NP %d\n",particle_count);
        #pragma acc loop independent default(present) reduction(+:color_val)
        for(int j = 0; j < particle_count; j++)
        {
            
            int index = particle_indexes[j];
            //printf("index %d\n",index);
            float distSq = 0.0f;
            #pragma acc loop default(present) reduction(+:distSq)
            for(int d = 0; d < dims; d++)
            {
                distSq += pow((particle_list[index].pos.v[d] - point.v[d])/particle_list[index].radius,2);
            }
            if(distSq < 1)
            {
                color_val += sqrt(1 - distSq);
            }
        }
        color_val = color_val - floor(color_val);
        free(particle_indexes);
        color_t color = (color_t){1.0f,color_val,color_val,color_val};
		PixelBuffer[i] = color_to_int(color);

	}
    //#pragma acc exit data delete(width, height, numParticles, particle_list[0:numParticles], Boxes[0:Boxes->numBoxes])
    //#pragma acc exit data copyout(PixelBuffer[0:width*height])
    free(Boxes);
}
