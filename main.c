#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "particle.h"
#include "BoundBox.h"
#include "physics.h"
#include "draw2D.h"
#include "bitmap.h"

int width = 1024;
int height = 1024;
int running = 1;

int main(){
    // init the things that are used forever!
    srand(time(NULL));
    int numParticles = 500;
    clock_t tick;
    clock_t tock;
    float timestep = 1.0f;
    
    unsigned int * PixelBuffer = (unsigned int *) malloc(sizeof(unsigned int) * width * height);
    char title[64];
    Particle * particle_list = init_particles(numParticles,(vec_t){0,0},(vec_t){width,height});
    
    int * sub_particles = (int *) malloc(sizeof(int) * numParticles);
    for(int i = 0; i < numParticles; i++)sub_particles[i] = i;
    int numFrame = 0;
    while (numFrame < 50) {
        printf("Frame : %d\n",numFrame);
        tick = clock();
        //Move particles by 1 frame's worth of time
        physics_step(particle_list, numParticles, sub_particles, timestep);
        tock = clock();
        
        printf("Physics Done in %ld\n",tock-tick);
        
        tick = clock();
        //draws the image
        draw_image(PixelBuffer, width, height, particle_list, numParticles, sub_particles);
        sprintf(title, "./Output/Frame_%d.bmp", numFrame++);
        save_bmp((unsigned char *)PixelBuffer, width, height, title);
        tock = clock();
        
        printf("Draw Done in %ld\n",tock-tick);
    }
    free(sub_particles);
    free(particle_list);
    free(PixelBuffer);
    

}
