/*
 * PROJECT 4: WYLIE GUNN AND DALLIN WILLIAMS
 */

#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include <altera_up_avalon_video_pixel_buffer_dma.h> // to swap front and back buffer
#include <math.h>  // for trigonometry functions
#include <stdlib.h>  // for file I/O
#include <altera_avalon_performance_counter.h>
#include "altera_avalon_mailbox.h"


alt_up_pixel_buffer_dma_dev *my_pixel_buffer; //declare global var
const int num_rows = 240;  //from 320
const int num_cols = 320; //from 240
float row;
float col;
int n = 4; //number of processors

//declaring mailboxes
alt_mailbox_dev* mailbox_0;
alt_mailbox_dev* mailbox_1;
alt_mailbox_dev* mailbox_2;
alt_mailbox_dev* mailbox_3;
alt_u32 message = 0;

void barrier(int id);
void pend(alt_mailbox_dev* box);
void post(int id, alt_u32 mess);


int main() {

	mailbox_0 = altera_avalon_mailbox_open("/dev/mailbox_0");
	mailbox_1 = altera_avalon_mailbox_open("/dev/mailbox_1");
	mailbox_2 = altera_avalon_mailbox_open("/dev/mailbox_2");
	mailbox_3 = altera_avalon_mailbox_open("/dev/mailbox_3");

	int cpuid = __builtin_rdctl(5);

	barrier(cpuid);
	alt_up_pixel_buffer_dma_dev *my_pixel_buffer; //declare global var
	my_pixel_buffer=alt_up_pixel_buffer_dma_open_dev("/dev/video_pixel_buffer_dma_0"); //assign it
	alt_up_pixel_buffer_dma_clear_screen(my_pixel_buffer,0);//clear_screen();

	int zoom = 1;
	float min_x, max_x, min_y, max_y;
	min_x = -2.5;
	max_x = 1.0;
	min_y = -1.0;
	max_y = 1.0;

	int i, j;
	float x, y, z;
	float x0, y0;
	float target_x, target_y, xtemp;
	int iteration;
	int is_target; //false
	int first_pass = 0; // on the first pass

	//barrier(cpuid);
	while (1) {
		is_target = 0;
		for (i = cpuid; i < num_rows; i+=4) {
			for (j = 0; j < num_cols; j++) {
				x = 0;
				y = 0;
				z = 0;
				x0 = j/320.0*(max_x - min_x) + min_x;
				y0 = (239.0-i)/240.0*(max_y - min_y) + min_y;
				iteration = 0;

				while( (x*x + y*y) <= 4 && iteration < 500) {
					xtemp = x*x - y*y + x0;
					y = 2*x*y + y0;
					x = xtemp;
					iteration++;

					if(iteration == 450 && first_pass == 0) {
						printf("FIRST PASS COMPLETE\n");
						first_pass = 1;
						if(cpuid == 0 && is_target == 0){
							printf("set target x and y for zoom \n");
							is_target = 1;
							target_x = x0;
							target_y = y0;
							//comment out from here to the other dotted line to get rid of the zoom stuff
							//if you run it with no zoom you'll notice I was able to adjust the barriers and
							//smooth out the drawing of the frame so that certain CPUs don't pull ahead
							//Zoom is still a problem (see the comment below)
							//*------------
							printf("target_x: %f...", target_x);
							printf("target_y: %f\n", target_y);
							//post target x and y values to the other mailboxes

							/*  Notes on some possible causes of the problems here
							 * * * * * * * * * *
							 * - CPU0 is correct - if you watch it draw the frames
							 *      after the first one, you'll see one out of every
							 *      4 lines is zoomed to the correct spot (it's
							 *      especially noticeable at the bottom)
							 *
							 * - That means the problem is in the way we pass the values to other CPUs:
							 *   - maybe need to typecast the values to alt_u32 before we pass them in
							 *   - maybe need to typecast the values when we read them out
							 * 	 - we should try to take a step back and see how casting between
							 * 		alt_u32s, ints, and floats works
							 *
							 *
							 *
							 */

							altera_avalon_mailbox_post(mailbox_1, target_x);
							altera_avalon_mailbox_post(mailbox_2, target_x);
							altera_avalon_mailbox_post(mailbox_3, target_x);
							altera_avalon_mailbox_post(mailbox_1, target_y);
							altera_avalon_mailbox_post(mailbox_2, target_y);
							altera_avalon_mailbox_post(mailbox_3, target_y);
							//pend mailbox 0 until the other 3 boxes have posted
							altera_avalon_mailbox_pend(mailbox_0);
							altera_avalon_mailbox_pend(mailbox_0);
							altera_avalon_mailbox_pend(mailbox_0);
						}
						if(cpuid == 1){
							printf("mailbox_1 pending\n");
							target_x = altera_avalon_mailbox_pend(mailbox_1);
							target_y = altera_avalon_mailbox_pend(mailbox_1);
							printf("target_x: %f...", target_x);
							printf("target_y: %f\n", target_y);
							altera_avalon_mailbox_post(mailbox_0, message);
						}
						if(cpuid == 2){
							printf("mailbox_2 pending\n");
							target_x = altera_avalon_mailbox_pend(mailbox_2);
							target_y = altera_avalon_mailbox_pend(mailbox_2);
							printf("target_x: %f...", target_x);
							printf("target_y: %f\n", target_y);
							altera_avalon_mailbox_post(mailbox_0, message);
						}
						if(cpuid == 3){
							printf("mailbox_3 pending\n");
							target_x = altera_avalon_mailbox_pend(mailbox_3);
							target_y = altera_avalon_mailbox_pend(mailbox_3);
							printf("target_x: %f...", target_x);
							printf("target_y: %f\n", target_y);
							altera_avalon_mailbox_post(mailbox_0, message);

							//---------*/
						}
					}

				}
				//printf("Iteration: %d\n", iteration);
				if(iteration == 500) {
					alt_up_pixel_buffer_dma_draw(my_pixel_buffer, 0,j,i );
				} else {
					alt_up_pixel_buffer_dma_draw(my_pixel_buffer, (iteration*8/zoom) + (iteration*4/zoom) +(iteration*2/zoom), j, i);
				}
				//barrier(cpuid);
			}
			barrier(cpuid);
		}

		barrier(cpuid);
		min_x = target_x - (1/(pow(1.5, zoom)));
		max_x = target_x + (1/(pow(1.5, zoom)));
		min_y = target_y - (0.75/(pow(1.5, zoom)));
		max_y = target_y + (0.75/(pow(1.5, zoom)));
		zoom += 1;

		alt_up_pixel_buffer_dma_clear_screen(my_pixel_buffer,0);
	}
	//barrier(cpuid);
}

void barrier(int id){
	post(id, message);
	if(id == 0){
		int i;
		for(i = 0; i < n-1; i++){
			pend(mailbox_0);
		}
	}
	if(id == 1){
		int i;
		for(i = 0; i < n-1; i++){
			pend(mailbox_1);
		}
	}
	if(id == 2){
		int i;
		for(i = 0; i < n-1; i++){
			pend(mailbox_2);
		}
	}
	if(id == 3){
		int i;
		for(i = 0; i < n-1; i++){
			pend(mailbox_3);
		}
	}
}
void pend(alt_mailbox_dev* box){
	//printf("MAILBOX PENDING \n");
	altera_avalon_mailbox_pend(box);
}

void post(int id, alt_u32 mess){
	 //altera_avalon_mailbox_post(box, message);

	 if(id == 0){
	 //printf("MAILBOX 0 POSTING \n");
	 altera_avalon_mailbox_post(mailbox_1, mess);
	 altera_avalon_mailbox_post(mailbox_2, mess);
	 altera_avalon_mailbox_post(mailbox_3, mess);
	 }
	 if(id == 1){
	 //printf("MAILBOX 1 POSTING \n");
	 altera_avalon_mailbox_post(mailbox_0, mess);
	 altera_avalon_mailbox_post(mailbox_2, mess);
	 altera_avalon_mailbox_post(mailbox_3, mess);
	 }

	 if(id == 2){
	 //printf("MAILBOX 2 POSTING \n");
	 altera_avalon_mailbox_post(mailbox_0, mess);
	 altera_avalon_mailbox_post(mailbox_1, mess);
	 altera_avalon_mailbox_post(mailbox_3, mess);
	 }

	 if(id == 3){
	 //printf("MAILBOX 3 POSTING \n");
	 altera_avalon_mailbox_post(mailbox_0, mess);
	 altera_avalon_mailbox_post(mailbox_1, mess);
	 altera_avalon_mailbox_post(mailbox_2, mess);
	 }

}

