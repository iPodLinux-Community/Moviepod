#ifndef MVPD_GLOBALS
#define MVPD_GLOBALS
#include <stdio.h>

#ifndef SOUND
#warning No Sound Subsystem
#endif


#define MVPD_FILE_VERSION 4
#define MVPD_PROGRAM_VERSION (0.0.6 ALPHA)

//These control the Amount of debug information
#define MVPD_DEBUG false
#define MVPD_DEBUG_LEVEL 0
#define MVPD_INSANE_DEBUG false

#ifdef FUNCTION_DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif


#ifdef DEBUG
FILE* debug_log;
#endif

#ifndef mvpd_uint32
typedef unsigned int   mvpd_uint32;
typedef   signed int    mvpd_int32;
typedef unsigned short mvpd_uint16;
typedef   signed short  mvpd_int16;
typedef unsigned char  mvpd_uint8;
typedef   signed char   mvpd_int8;
#endif

#ifndef true
typedef signed char bool;
#define true 1
#define false 0
#endif

#define inl(x) (*(volatile unsigned long *)(x))

//Used for the Quantization Table
#define LUMINANCE 0
#define CHROMINANCE 1

#define MVPD_END_OF_PICTURE_FRAME ((mvpd_int8) (127))
#define MVPD_END_OF_PICTURE_COLOUR ((mvpd_int8) (126))
#define MVPD_END_OF_PICTURE_BLOCK ((mvpd_int8) (125))

#define MVPD_END_OF_AUDIO_FRAME ((mvpd_int8) (127))

#define MVPD_PLAYER_FILE_NOT_FOUND ((mvpd_uint8) (44))
#define MVPD_PLAYER_FILE_CORRUPT ((mvpd_uint8) (100))
#define MVPD_PLAYER_FILE_FINE ((mvpd_uint8) (1))


extern void MVPD_INIT_DECODING();
extern void MVPD_CLOSE_DECODING();

/* These are the functions for converting from one colorspace to another and
 * up/downsampling an array. These functions where formerly in the file
 * conversion.h */

/* Converts an array of RGB 888 values to YUV 888 values and writes the output into the RGB Matrix */
extern void MVPD_RGB_ARRAY_TO_YUV_ARRAY(mvpd_uint8 RGB[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

/* Converts an array of YUV 888 values to RGB 888 values and writes the output into the YUV Matrix */
extern void MVPD_YUV_ARRAY_TO_RGB_ARRAY(mvpd_uint8 YUV[], mvpd_uint16 matrix, mvpd_uint16 matrix_y);

/* Converts an array of YUV 888 values to RGB 565 values and writes the output into the RGB Matrix */
extern void MVPD_YUV_ARRAY_TO_RGB565_ARRAY(mvpd_uint8 YUV[], mvpd_uint16 length, mvpd_uint16 RGB[]);

/* Takes an YUV 888 Array and writes the output into the arrays Y,U,V with U and V downsampled to (matrix_y / 2) x (matrix_x / 2) */
extern void CHROMA_SUBSAMPLING(mvpd_uint8 input[], mvpd_int16 Y[], mvpd_int16 U[], mvpd_int16 V[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

extern void REVERSE_CHROMA_SUBSAMPLING(mvpd_uint8 Y[], mvpd_uint8 U[], mvpd_uint8 V[], mvpd_uint8 output[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);


/* These are the functions for used to perform the discrete cosine tranformation
 * These functions where formerly in the file dct.h
 */
/* Shifts the array down by 128, this function is still used for encoding the image*/
extern void MVPD_SHIFT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);
/* Shifts the array up by 128, this function is not used anymore*/
extern void MVPD_ISHIFT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

/* Performs the dct on the input array. Still used in the encoding process*/
extern void MVPD_DCT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);
/* Reverses the dct on the input array. Is not used anymore, instead the function MVPD_BLOCK_IDCT
 * is called directly from MVPD_IZIGZAG */
extern void MVPD_IDCT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

/* Reverses the dct on an 8x8 array. This function is called directly from MVPD_IZIGZAG */
extern void MVPD_BLOCK_IDCT(mvpd_int16 input[]);


/* These functions are used to organise the Results from the dct in an List and
 * divide this list by the quantization matrix.
 * These functions where formerly in the file quant.h
 */
/* Adjusts the quantisation matrix for the given quality */
extern void MVPD_INITIALIZE_QUANT(mvpd_uint8 quality);

/* Organises the values from input in an zigzag order and divides them by the quantization matrix.
 * The value chrom indicates, wether the chrominance quantization table or the luminance table
 * should be used.*/
extern mvpd_uint16 MVPD_ZIGZAG(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y, mvpd_uint8 chrom, mvpd_int8 output[], mvpd_uint32 position);
/* To increase the performance this function reverses the quantization, zigzag order and dct in one go.
 * The value chrom indicates, wether the chrominance quantization table or the luminance table
 * should be used.*/
extern mvpd_uint16 MVPD_IZIGZAG(mvpd_int8 input[], mvpd_uint32 position, mvpd_uint8 chrom, mvpd_uint8 output[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

extern void _MVPD_ARM_COLUMN_DCT(mvpd_int16* input, mvpd_int16* output);
extern void _MVPD_ARM_ROW_DCT(mvpd_int16* input, mvpd_int16* output);

extern void MVPD_CLIP(mvpd_uint8* input, mvpd_uint16 input_x, mvpd_uint16 input_y, mvpd_uint8* output, mvpd_uint16 output_x, mvpd_uint16 output_y);

/* These are temporary functions for reading bitmaps. When avi support is completed
 * these shouldn't be used anymore. These functions where formerly in the file bmp.h
 */
     typedef struct {
  mvpd_uint16 height, width;
  mvpd_uint8 * picture;
} BMP;

extern void INIT_BITMAP(BMP* bitmap);
extern void LOAD_BITMAP(char* filename, BMP* bitmap, mvpd_uint16 matrix_x, mvpd_uint16 matrix_y);

/***************************************************************
 *            Audio Stuff                                      *
 ***************************************************************/

/* These are functions used for reading and converting wav data.
 * These functions were formerly in the file wav.h.
 */

typedef struct
{
  char* filename;
  FILE* wav_file;
  mvpd_uint8 channels;
  mvpd_uint32 sample_rate;
  mvpd_uint32 bytes_per_sec;
  mvpd_uint16 block_align;
  mvpd_uint16 bits_per_sample;
  mvpd_uint32 size_audio_data;
  mvpd_uint32 position;
  mvpd_uint32 fragment_size;
} WAVE;

extern mvpd_int8 MVPD_OPEN_WAVE(char* filename, WAVE* wave);
extern void MVPD_CLOSE_WAVE(WAVE* wave);

extern mvpd_uint32 MVPD_INIT_WAVE(WAVE* wave, mvpd_uint32 fragment_size);
extern mvpd_uint32 MVPD_LOAD_WAVE_DATA(mvpd_int16* audio_buffer, mvpd_uint32 buffer_size, WAVE* wave);

extern mvpd_int8 MVPD_COMPRESS_AUDIO(mvpd_int16* input, mvpd_uint32 length, mvpd_int8* output);
extern mvpd_int8 MVPD_DECOMPRESS_AUDIO(mvpd_int8* input, mvpd_uint32 length, mvpd_int16* output);
extern mvpd_int8 MVPD_DECOMPRESS_AUDIO2(mvpd_int8* input, mvpd_uint32 length, mvpd_int16* output);

extern mvpd_int8 MVPD_INFLATE_AUDIO(mvpd_int16* input, mvpd_uint32 length);

extern mvpd_int8 MVPD_INIT_AUDIO(mvpd_uint16 fragment_size, mvpd_uint8 bitrate, mvpd_uint8 channels, mvpd_uint16 sample_rate);

extern void MVPD_INCREASE_VOLUME();
extern void MVPD_DECREASE_VOLUME();

extern void MVPD_STOP_AUDIO();

/***************************************************************
 *            Input Stuff                                      *
 ***************************************************************/

/* Just for geting Input */

#define  KEYCODE(a) (a & 0x7f)  // Use to get keycode of scancode.

#define KH_KEY_MENU    50
#define KH_KEY_REWIND  17
#define KH_KEY_PLAY    32
#define KH_KEY_FORWARD 33

#define KH_KEY_ACTION  28
#define KH_KEY_HOLD    35

#define KH_WHEEL_L     38
#define KH_WHEEL_R     19

extern int MVPD_START_INPUT(void);
extern void MVPD_STOP_INPUT(void);
extern int MVPD_GET_KEY(void);
#endif
