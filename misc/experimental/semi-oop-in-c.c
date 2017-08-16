#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define NOT_USED 0

int calculateVolume(int l,int w,int h){
	
	return l*w*h;
}

/* self-referential structure */
typedef struct Boxs {            
	int length;
	int width;
	int height; 
	int (*calculateVolume)(int,int,int); 
} Box; /* end structure listNode */

int main()
{ 
	Box first_box;
	first_box.calculateVolume = &calculateVolume;
	first_box.length = 10;
	first_box.width = 12;
	first_box.height = 5;
	printf("Volume is %d", first_box.calculateVolume(first_box.length, first_box.width, first_box.height));
    return 0; /* indicates successful termination */

} /* end main */
