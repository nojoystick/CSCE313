/*
 * PROJECT 4: WYLIE GUNN AND DALLIN WILLIAMS
 */

#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include <altera_up_avalon_video_character_buffer_with_dma.h>  // to write characters to video
#include <altera_up_avalon_video_pixel_buffer_dma.h> // to swap front and back buffer
#include <math.h>  // for trigonometry functions
#include <stdlib.h>  // for file I/O
#include <altera_avalon_performance_counter.h>
//#include "altera_avalon_mailbox.h"


alt_up_pixel_buffer_dma_dev *my_pixel_buffer; //declare global var
const int num_rows = 240;  //from 320
const int num_cols = 320; //from 240
float row;
float col;

int main()
{
/*
	//declaring mailboxes
	alt_u32 message = 0;
	alt_mailbox_dev* mailbox_0;
	alt_mailbox_dev* mailbox_1;

	mailbox_0 = altera_avalon_mailbox_open("/dev/mailbox_0");
	mailbox_1 = altera_avalon_mailbox_open("/dev/mailbox_1");

	int cpuid = __builtin_rdctl(5);

	//BARRIER: DO WE NEED THIS HERE?
	/*
	if(cpuid == 0) {
		altera_avalon_mailbox_post(mailbox_1, message);
		altera_avalon_mailbox_pend(mailbox_0);
	} else {
		altera_avalon_mailbox_post(mailbox_0, message);
		altera_avalon_mailbox_pend(mailbox_1);
	}
*/

	//pixel buffer

	//alt_up_pixel_buffer_dma_dev *my_pixel_buffer; //declare global var
	my_pixel_buffer=alt_up_pixel_buffer_dma_open_dev("/dev/video_pixel_buffer_dma_0"); //assign it
	alt_up_pixel_buffer_dma_clear_screen(my_pixel_buffer,0);//clear_screen();

	int zoom = 1;
	float min_x, max_x, min_y, max_y;
	min_x = -2.5;
	max_x = 1.0;
	min_y = -1.0;
	max_y = 1.0;
	while (1) {
		int i, j;
		float x, y, z;
		float x0, y0;

		float target_x, target_y;
		int iteration;
		int is_target = 0; //false

		for (i = 0; i < num_rows; i++) {
			for (j = 0; j < num_cols; j++) {
				x = 0;
				y = 0;
				z = 0;
				x0 = j/320.0*(max_x - min_x) + min_x;
				y0 = (239.0-i)/240.0*(max_y - min_y) + min_y;
				//printf("x0: %f...", x0);
				//printf("y0: %f\n", y0);
				iteration = 0;

				while( (x*x + y*y) <= 4 && iteration < 500) {
					float xtemp = x*x - y*y + x0;
					y = 2*x*y + y0;
					x = xtemp;
					iteration++;
					if(iteration == 450 && is_target == 0) {
						is_target == 1;
					}
				}
				if(is_target == 1) {
					target_x = x0;
					target_y = y0;
				}

				//printf("Iteration: %d\n", iteration);
				if(iteration == 500) {
					alt_up_pixel_buffer_dma_draw(my_pixel_buffer, 0,j,i );
				} else {
					alt_up_pixel_buffer_dma_draw(my_pixel_buffer, (iteration*8/zoom) + (iteration*4/zoom) +(iteration*2/zoom), j, i);
				}

			}
		}
		min_x = target_x - 1/(pow(1.5, zoom));
		max_x = target_x + 1/(pow(1.5, zoom));
		min_y = target_y - 0.75/(pow(1.5, zoom));
		max_y = target_y + 0.75/(pow(1.5, zoom));
		zoom += 1;
		alt_up_pixel_buffer_dma_clear_screen(my_pixel_buffer,0);
	}
}
