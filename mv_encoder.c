#include <stdlib.h>
#include <stdio.h>

#include "mvpd.h"




mvpd_uint32 START_FRAME = 0;
mvpd_uint32 END_FRAME = 0;
mvpd_uint8 QUALITY = 11;
mvpd_uint8 COLOR = 1;
mvpd_uint8 AUDIO = 1;
mvpd_uint8 MS_PER_FRAME = 100;
mvpd_uint16 MATRIX_Y = 144;
mvpd_uint16 MATRIX_X = 176;
mvpd_uint16 SCREEN_Y = 132;
mvpd_uint16 SCREEN_X = 176;

char* FILE_PREFIX="";
char* AUSGABE_NAME="test.mvpd";


//------------------------ Argument Handler -----------------------
// Returns false, if an invalid argument was passed
bool argHand(int argc, char *argv[])
{
  mvpd_int32 i;
  for(i = 1; i < argc; i++)
  {
    if(argv[i][0] != '-')
    {
      printf("%s is not a valid argument!\nThe program will be terminated.\n", argv[i]);
      return false;
    }
    switch ( argv[i][1] )
    {
      case 's':
        START_FRAME = atoi(argv[i+1]);
        printf("\nStartframe: %i\n", START_FRAME);
        i++;
        break;
      case 'e':
        END_FRAME = atoi(argv[i+1]);
        printf("\nEndframe: %i\n", END_FRAME);
        i++;
        break;
      case 'f':
        FILE_PREFIX = (argv[i+1]);
        printf("\nInputprefix: %s\n", FILE_PREFIX);
        i++;
        break;
      case 'o':
        AUSGABE_NAME = (argv[i+1]);
        printf("\nOutputfile: %s\n", AUSGABE_NAME);
        i++;
        break;
      case 'q':
        QUALITY = atoi(argv[i+1]);
        printf("\nQuality: %i\n", QUALITY++);
        i++;
        break;
      case 'b':
        COLOR = 0;
        printf("\nBlack and White Mode");
        break;
      case 'a':
        AUDIO = 0;
        printf("\nNo Audio");
        break;
      case 'x':
        SCREEN_X = atoi(argv[i+1]);
        printf("\nX: %i\n", SCREEN_X);
        i++;
        break;
      case 'y':
        SCREEN_Y= atoi(argv[i+1]);
        printf("\nX: %i\n", SCREEN_Y);
        i++;
        break;
      case 'm':
        MS_PER_FRAME = atoi(argv[i+1]);
        printf("\nMilisecs / Frame: %i\n", MS_PER_FRAME);
        i++;
        break;

      case '-':
        if (argv[i] == "--help")
        {
          printf("\nUsage: %s -s [startframe] -e [endframe] -f [eingabedatei] -o [ausgabedatei]\n", argv[0]);
          break;
        }
        if (argv[i] == "--file")
        {
          FILE_PREFIX = (argv[i+1]);
          i++;
          break;
        }
        break;

    }
  }
  fflush(stdout);
  return true;
}


//------------------------ Main Program -----------------------

int main(int argc, char *argv[]) {

  FILE_PREFIX =(char*) calloc(80, sizeof(char));
  AUSGABE_NAME =(char*) calloc(80, sizeof(char));
  AUSGABE_NAME="test.mvpd";
  fpos_t max_length_pos;
  fpos_t frame_index_pos;


  if (!(argHand(argc, argv)))
    return 1;
  
  MATRIX_X = SCREEN_X;
  MATRIX_Y = SCREEN_Y;
  
  if((MATRIX_X % 16) != 0)
    MATRIX_X = SCREEN_X + ( 16 - (SCREEN_X % 16));
  if((MATRIX_Y % 16) != 0)
    MATRIX_Y = SCREEN_Y + ( 16 - (SCREEN_Y % 16));
  if (true)
  {
    printf("\nOriginal X: %i -> Adapted X: %i\nOriginal Y: %i -> Adapted Y: %i", SCREEN_X, MATRIX_X, SCREEN_Y, MATRIX_Y);
    fflush(stdout);
  }
  
  mvpd_uint32 i = 0, j = 0;

  mvpd_uint8 bild[MATRIX_Y*MATRIX_X*3];
  mvpd_int16 DCT_Y[MATRIX_Y*MATRIX_X];
  mvpd_int16 DCT_U[(MATRIX_Y/2)*(MATRIX_X/2)];
  mvpd_int16 DCT_V[(MATRIX_Y/2)*(MATRIX_X/2)];
  mvpd_int8 LISTE[MATRIX_Y * MATRIX_X * 4]; //Should be enough to hold all possible values
  mvpd_uint16 position = 0;

  mvpd_int16* uncompressed_sound = NULL;
  mvpd_int8* compressed_sound = NULL;


  signed char data;
  char mvpd_intbuffer[8];
  char video_filename[80];
  mvpd_uint32 start_offset = 29 + ((END_FRAME - START_FRAME + 1) 
* 4);
  mvpd_uint32 total_frames = END_FRAME - START_FRAME + 1;
  mvpd_uint16 max_frame_length = 0;



  WAVE* wav = NULL;
  mvpd_uint16 fragment_size = 0;
  mvpd_uint8 bitrate = 0;
  mvpd_uint8 stereo = 0;
  mvpd_uint16 sample_rate = 0;

  BMP* bitmap;
  bitmap = (BMP*) malloc(sizeof(BMP));
  bitmap->height = MATRIX_Y;
  bitmap->width = MATRIX_X;
  INIT_BITMAP(bitmap);

  mvpd_uint32 frame_index[(END_FRAME - START_FRAME + 1)];
  
  /*Audio Stuff */
  if(AUDIO)
  {
    sprintf(video_filename,"%s.wav", FILE_PREFIX);
    int check;
    wav = (WAVE*) malloc(sizeof(WAVE));
    if((check = MVPD_OPEN_WAVE(video_filename, wav)) != 0)
    {
      printf("\nError while opening wave file!\n");
      return 0;
    }

    if(abs((wav->size_audio_data / wav->bytes_per_sec) - ((total_frames * MS_PER_FRAME) / 1000)) > 2)
    {
      printf("\nError length of Audio: %u  length of video: %u",(wav->size_audio_data / wav->bytes_per_sec), ((total_frames * MS_PER_FRAME) / 1000));
      MS_PER_FRAME = (wav->size_audio_data / wav->bytes_per_sec) / total_frames;
      printf("\nAdjusted video speed to %u ms / frame.",MS_PER_FRAME);
    }
    fragment_size = (wav->channels * ( (wav->sample_rate * MS_PER_FRAME) / 1000)); //The amount of Samples per frame

    bitrate = wav->bits_per_sample;
    stereo = wav->channels -1;
    sample_rate = wav->sample_rate;

    uncompressed_sound = (mvpd_int16*) calloc(sizeof(mvpd_int16), 10240);
    compressed_sound = (mvpd_int8*) calloc(sizeof(mvpd_int8), 10240);

  }
  /*Ende audio Stuff */
  
  start_offset += (10240 - fragment_size) + 10240;
  
  
  for(i = 0; i < (MATRIX_Y * MATRIX_X * 3); i++)
  {
    bild[i] = 0;
    LISTE[i] = 0;

    DCT_Y[i / 3] = 0;
    DCT_U[i / 12] = 0;
    DCT_V[i / 12] = 0;
  }


  FILE *ausgabedatei = fopen(AUSGABE_NAME, "wb");
  if (ausgabedatei == NULL)
  {
    printf("\nSorry, can't create the file %s.\nThe program will be terminated.\n", AUSGABE_NAME);
    return -1;
  }

  // Header Schreiben
  fputc((unsigned char)('M'), ausgabedatei);
  fputc((unsigned char)('V'), ausgabedatei);
  fputc((unsigned char)('P'), ausgabedatei);
  fputc((unsigned char)('D'), ausgabedatei);

  fputc((unsigned char)(MVPD_FILE_VERSION), ausgabedatei);

  printf("\nStart Offset: %i", start_offset);
  if(AUDIO == 0)
    start_offset -= 6;
  printf("\nStart Offset: %i", start_offset);
  fwrite(&start_offset, sizeof(start_offset), 1, ausgabedatei);

  fwrite(&MATRIX_X, sizeof(MATRIX_X), 1, ausgabedatei);

  fwrite(&MATRIX_Y, sizeof(MATRIX_Y), 1, ausgabedatei);

  fgetpos( ausgabedatei, &max_length_pos );
  fwrite(&max_frame_length, sizeof(max_frame_length), 1, ausgabedatei);
  
  
  fwrite(&QUALITY, sizeof(QUALITY), 1, ausgabedatei);
  if(COLOR)
    fputc((unsigned char)('c'), ausgabedatei);
  else
    fputc((unsigned char)('b'), ausgabedatei);

  total_frames = END_FRAME - START_FRAME + 1;
  fwrite(&total_frames, sizeof(total_frames), 1, ausgabedatei);

  fwrite(&MS_PER_FRAME, sizeof(MS_PER_FRAME), 1, ausgabedatei);

  fwrite(&AUDIO, sizeof(AUDIO), 1, ausgabedatei);
  if(AUDIO)
  {
    fwrite(&fragment_size, sizeof(fragment_size), 1, ausgabedatei);
    fwrite(&bitrate, sizeof(bitrate), 1, ausgabedatei);
    fwrite(&stereo, sizeof(stereo), 1, ausgabedatei);
    fwrite(&sample_rate, sizeof(sample_rate), 1, ausgabedatei);
  }
  
  fgetpos( ausgabedatei, &frame_index_pos );
  for(i = 0; i < (END_FRAME - START_FRAME); i++)
  {
    frame_index[i] = 0;
    fwrite(&frame_index[i], sizeof(mvpd_int32), 1, ausgabedatei);
  }
  fputc((unsigned char)(0xDE), ausgabedatei);
  fputc((unsigned char)(0xAD), ausgabedatei);
  fputc((unsigned char)(0xDA), ausgabedatei);
  fputc((unsigned char)(0x7A), ausgabedatei);
  
  if(AUDIO)
  {
    MVPD_LOAD_WAVE_DATA(uncompressed_sound, (10240 * 2), wav);
    MVPD_COMPRESS_AUDIO(uncompressed_sound,  10240, compressed_sound);
    if(10240 != (fwrite(compressed_sound, sizeof(mvpd_int8), 10240, ausgabedatei)))
      return -1;
    
    MVPD_LOAD_WAVE_DATA(uncompressed_sound, ((10240 - fragment_size) * 2), wav);
    MVPD_COMPRESS_AUDIO(uncompressed_sound, (10240 - fragment_size), compressed_sound);
    if((10240 - fragment_size) != (fwrite(compressed_sound, sizeof(mvpd_int8), (10240 - fragment_size), ausgabedatei)))
      return -1;
  }
  
  
  MVPD_INITIALIZE_QUANT(QUALITY);

  for(i = START_FRAME; i <=END_FRAME; i++)
  {
    if((i - START_FRAME) % 1000 == 0)
      printf("\nProcessing Frame %i of %i", (i - START_FRAME), (END_FRAME - START_FRAME));

    frame_index[i-START_FRAME] = ftell( ausgabedatei );
#ifdef DEBUG
    printf("\nFrame %i\t Offset: %i", i, frame_index[i]);
#endif

    if(AUDIO)
    {
      MVPD_LOAD_WAVE_DATA(uncompressed_sound, ((fragment_size * wav->bits_per_sample) / 8), wav);
      MVPD_COMPRESS_AUDIO(uncompressed_sound, fragment_size, compressed_sound);
//      DIFF_ENCODING(compressed_sound, fragment_size, uncompressed_sound);
      if((fragment_size) != (fwrite(compressed_sound, sizeof(mvpd_int8), fragment_size, ausgabedatei)))
        return -1;
      data = MVPD_END_OF_AUDIO_FRAME;
      putc(data, ausgabedatei);
    }

    sprintf(mvpd_intbuffer,"%i", i);

    sprintf(video_filename,"%s%s.bmp", FILE_PREFIX, mvpd_intbuffer);
#ifdef DEBUG
      printf("\nProcessing file: %s", video_filename);
#endif

    LOAD_BITMAP(video_filename, bitmap, MATRIX_X, MATRIX_Y);
    for(j = 0; j < (MATRIX_Y * MATRIX_X * 3); j++)
      bild[j] = 0;
    for(j = 0; j < (MATRIX_Y * MATRIX_X * 3); j++)
      bild[j] = bitmap->picture[j];

    MVPD_RGB_ARRAY_TO_YUV_ARRAY(bild, MATRIX_X, MATRIX_Y);

    CHROMA_SUBSAMPLING(bild, DCT_Y, DCT_U, DCT_V, MATRIX_X, MATRIX_Y);

    MVPD_SHIFT(DCT_Y, MATRIX_X, MATRIX_Y);
    if(COLOR)
    {
      MVPD_SHIFT(DCT_U, (MATRIX_X / 2), (MATRIX_Y / 2));
      MVPD_SHIFT(DCT_V, (MATRIX_X / 2), (MATRIX_Y / 2));
    }

    MVPD_DCT(DCT_Y, MATRIX_X, MATRIX_Y);
    if(COLOR)
    {
      MVPD_DCT(DCT_U, (MATRIX_X / 2), (MATRIX_Y / 2));
      MVPD_DCT(DCT_V, (MATRIX_X / 2), (MATRIX_Y / 2));
    }

    position = 0;
    position = MVPD_ZIGZAG(DCT_Y, MATRIX_X, MATRIX_Y, LUMINANCE, LISTE, (position = 0));
    if(COLOR)
    {
      position = MVPD_ZIGZAG(DCT_U, (MATRIX_X / 2), (MATRIX_Y / 2), CHROMINANCE, LISTE, position);
      position = MVPD_ZIGZAG(DCT_V, (MATRIX_X / 2), (MATRIX_Y / 2), CHROMINANCE, LISTE, position);
    }

#ifdef DEBUG
     printf("\nWriting to file.");
#endif

    if(position > max_frame_length)
      max_frame_length = position;
    fwrite(&position, sizeof(position), 1, ausgabedatei);

#ifdef DEBUG
      printf("\nLaenge: %i.", position);
#endif

    if((position) != (fwrite(LISTE, sizeof(mvpd_int8), position, ausgabedatei)))
      return -1;
    data = MVPD_END_OF_PICTURE_FRAME;
    putc(data, ausgabedatei);

  }
  putc(0, ausgabedatei);
  
  fsetpos( ausgabedatei, &max_length_pos );
  fwrite(&max_frame_length, sizeof(max_frame_length), 1, ausgabedatei);
  
  
  fsetpos( ausgabedatei, &frame_index_pos );
  for(i=0; i < (END_FRAME - START_FRAME); i++)
  {
    fwrite(&frame_index[i], sizeof(mvpd_uint32), 1, ausgabedatei);
  }

  fclose(ausgabedatei);
  printf("\n");
  return 0;
}

