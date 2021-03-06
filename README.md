*********************************************************************************
Please note:
Moviepod is designed as a proof of concept!
At the current level the program is not intended for normal use!
*********************************************************************************

Moviepod is an encoder, decoder setup for the propietary mvpd file format.
The moviepod file format was created with the intention to make the decoding
process as fast as possible and to minimize the risk of desynchronization between
audio and video data.



*********************************************************************************
Please note:
The purpose of this project was to learn about iPod programing,
multimedia programing, compression algorithms etc.
This is in many ways my first real project, therefore there are a lot of things
which might seem rather complicated or idiotic in retrospective.
Also this project is still a work in progress, there are a lot of things, that i
want to change or rewrite, but that i didn't have time for so far.
*********************************************************************************

Files:
The following files should be in the archive you downloaded.

audio.c		- Contains all audio related functions.
		  Mainly the initialization function for the soundcard,
		  the functions used to read a .wav file and the 
		  functions used for shifting, compressing and upsampling
		  the audio data.
		  
Bugs.txt	- List of some known bugs, by far not all since there are
		  currently too many to list them all.
		  
documentation.txt The file you are reading right now.

fileformat.txt	- Short overview of the mvpd file format.

Makefile	- A very basic Makefile, if you want to build Moviepod on
		  your own, you will have to edit the variable Hotdog_Path
		  More probably you might want to rewrite the whole thing
		  from scratch.
		  
mv_analyzer.c	- The source code for an analyzer program.
		  This program outputs some general information about a
		  .mvpd file and should find most obvious file errors.
		  
mv_encoder.c	- The source code for the mvpd encoder.
		  For general usage information see below.
		  
mv_player.c	- The source code for the mvpd player.
		  The source code is valid for both a desktop
		  build of mvpd and an iPod build.
		  For the desktop build it uses SDL to display the image data.
		  
mvpd.h		- This header contains all defines, data types and prototypes
		  for all functions defined in audio.c and video.c.
		  If you for some reason want to incorperate these functions
		  in your own program, include this file.
		  
Todo.txt	- A list of a few features, that i want to include before the
		  Beta release.
		  
video.c		- Contains all functions related to loading bmp files and 
		  compressing / converting images.

		  
A general note regarding the source code:
All functions are all upercase and prefixed with MVPD_.
All global variables should be all upercase too, while a local variables should be all lowercase.
(This is currently not the case, though)


//The Encoder:
The included encoder is only intented for testing purposes, since I didn't like idea of understanding
the avi file format at the beginning of the project and I had the bmp code already lying around.

The current encoder reads in a series of bmp files and a wav file and creates a mvpd file out of these.

possible switches are:

-s [start_frame]	The number of the first bitmap to be processed
			Default value: 0

-e [end_frame] 		The number of the last bitmap to be processed
			Default value: 0

-f [inputprefix]	The prefix for each Bitmap file this string is put
			in front of the current frame number and used as inputfile.
			Default value: empty

-o [outputfile]		The file name for the finale movie.
			Default value: 'test.mvpd'
			
-q [quality]		The quality setting with which the movie should be encoded.
			influences the quantization table.
			Can range between 0 and 255, with 0 being highest quality
			and 255 lowest.
			Default value: 10
			
-x [width]		The movie width.
			If the input bitmap is bigger than this, the right side
			of the image will be cut of.
			If the input bitmap is smaller than this, the frame will be
			filled with black.
			Default value: 176
			
-y [height]		The movie height.
			If the input bitmap is bigger than this, the upper part
			of the image will be cut of.
			If the input bitmap is smaller than this, the frame will be
			filled with black.
			Default value: 132

-m [ms_per_frame]	The time each image should be displayed.
			If audio is activated, this value will be adjusted to fit the
			audio length.
			This value is currently ignored!
			Default value: 100
			
-b 			Black and white mode
			If this switch is passed the video will be safed in grayscale mode.
			Default value: off
			
-a			No Audio
			If this switch is passed no audio data will be safed in the video.
			Default value: off
			
Example:
if you want to encode the movie cruel intentions (Which you have to own, if you want to do this)
for your iPod Photo, you have to:

- Use an external program to:
	1. Change the frame rate of the movie (10 fps is usually a good value)
	2. Save the audio in a wav file (At the moment only 16 bit 44.1kHz pcm is supported)
	3. Change the video size to 220 x 176 (Photo resolutio)
	4. Extract the individual frames from the video file into one folder.
	   (At 10 fps this should be ~48 000 files)
	   
You should name the bmps something like intentions0.bmp - intentions47999.bmp
And the wave file accordingly intentions.wav

- Then you start the encoder with
./mv_encoder -s 0 -e 47999 -f intentions -o cruel_intentions.mvpd -q 15 -x 220 -y 176 -m 100

The encoding process can take up to one hour, since none of the functions are optimiued for speed.
Afterwards you can check the quality with:
./mv_player cruel-intentions.mvpd
