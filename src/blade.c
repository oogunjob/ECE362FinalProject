/* GIMP RGB C-Source image dump (blade.c) */

//displays a very small 5x5 blue/grey ball (aka "blade") to represent the current
//position of the touch on the screen. Ideally has a tail made up of carbon
//copies of the original blade image

const struct {
  unsigned int 	 width;
  unsigned int 	 height;
  unsigned int 	 bytes_per_pixel; /* 2:RGB16, 3:RGB, 4:RGBA */ 
  unsigned char	 pixel_data[5 * 5 * 2 + 1];
} blade = {
  5, 5, 2,
  "\377\377\034\337\323\214\377\377\377\377\226\245f\011\206\011e\011Qt\010\"\246"
  "\021e\011\206\021\347\031\317ce\011\313:\246\021MS\377\377\226\245\252:\024\225"
  "\337\377",
};

