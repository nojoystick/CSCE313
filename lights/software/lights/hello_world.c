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
#include "altera_avalon_mailbox.h"


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


	while (1) {
		int i, j;
		for (i = 0; i < num_rows; i++) {
			for (j = 0; j < num_cols; j++) {
				alt_up_pixel_buffer_dma_draw(my_pixel_buffer, (i*320*3) + (i*320*3<<8) + (i*320*3<<16),j,i );
			}
		}
		alt_up_pixel_buffer_dma_clear_screen(my_pixel_buffer,0);
	}
}
