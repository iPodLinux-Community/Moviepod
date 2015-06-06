#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include "mvpd.h"

// Entry point
int main(int argc, char *argv[])
{

  mvpd_uint8 header_buffer[4];

  mvpd_int8 s_char;
  mvpd_uint8 u_char;
  mvpd_uint8 quality;
  mvpd_uint8 color = 0;
  mvpd_uint8 ms_per_frame;
  mvpd_uint8 file_version;
  mvpd_uint32 total_frames = 0;
  mvpd_uint32 length = 0;
  mvpd_uint16 expected_frame_length = 0;
  mvpd_uint32 total_length = 0;
  mvpd_uint32 frames = 0;
  mvpd_uint32 expected_blocks = 0;
  mvpd_uint32 max_length = 0;
  mvpd_uint16 expected_max_frame_length = 0;
  mvpd_uint32 min_length;
  mvpd_uint32 start_offset = 0;
  mvpd_uint16 x,y;
  mvpd_uint16 blocks;
  bool detailed = false;

  //Audio variables:
  mvpd_uint8 audio;
  mvpd_uint16 expected_fragment_size = 0;
  mvpd_uint16 actual_fragment_size = 0;
  mvpd_uint8 bitrate = 0;
  mvpd_uint8 stereo = 0;
  mvpd_uint16 sample_rate = 0;
  mvpd_uint16 audio_bytes = 0;


  FILE *inputfile = fopen(argv[1], "rb");

  if (inputfile == NULL)
  {
    printf("\nSorry, can't open the file %s.\nThe program will be terminated.\n", argv[1]);
    return -1;
  }


  fread(header_buffer, 1, 4, inputfile);
  if((header_buffer[0] == 'M') && (header_buffer[0] == 'V') && (header_buffer[0] == 'P') && (header_buffer[0] == 'D'))
  {
    printf("\nHeader error, first bytes should be MVPD, but are %s.", header_buffer);
    fflush(stdout);
  }

  file_version = getc(inputfile);
  printf("\nFile Version: %i", file_version);

  fread(&start_offset, sizeof(start_offset), 1, inputfile);

  fread(&x, sizeof(x), 1, inputfile);
  fread(&y, sizeof(y), 1, inputfile);
  if(file_version > 1)
    fread(&expected_max_frame_length, sizeof(expected_max_frame_length), 1, inputfile);

  quality = getc(inputfile);
  u_char = getc(inputfile);
  if(u_char == 'c')
    color = 1;
  else if(u_char == 'b')
    color = 0;
  else
  {
    printf("\nError wrong color indicator!");
  }

  fread(&total_frames, sizeof(total_frames), 1, inputfile);

#ifdef DEBUG
    printf("\nTotal number of frames: %i", total_frames);
#endif

  ms_per_frame = getc(inputfile);

  if(color == 1)
    expected_blocks = ((((x*y) / 64) * 3) / 2);
  else
    expected_blocks = ((x*y) / 64);

  printf("\nErwartete Blockzahl: %i", expected_blocks);
  fflush(stdout);

  min_length = x * y * 3 + expected_blocks + 1;

//Neuer Teil:
  if(file_version > 0)
  {
    fread(&audio, sizeof(audio), 1, inputfile);
    if(audio)
    {
      fread(&expected_fragment_size, sizeof(expected_fragment_size), 1, inputfile);
      fread(&bitrate, sizeof(bitrate), 1, inputfile);
      fread(&stereo, sizeof(stereo), 1, inputfile);
      fread(&sample_rate, sizeof(sample_rate), 1, inputfile);
      if(abs(((expected_fragment_size * 1000) / (sample_rate * (stereo + 1))) - ms_per_frame) > 100);
      printf("\n\nFragmentsize doesn't fit video speed:\nFragment size: %i\nMiliseconds per picture: %i", expected_fragment_size, ms_per_frame);
    }
  }





  fseek(inputfile, start_offset, SEEK_SET);
  while (feof(inputfile) == 0)
  {
    blocks = 0;
    length = 0;
    
    //Audio Stuff:
    if(audio)
    {
      actual_fragment_size = 0;
      s_char = getc(inputfile);
      while((s_char != MVPD_END_OF_AUDIO_FRAME) && (feof(inputfile) == 0))
      {
        actual_fragment_size++;
        audio_bytes++;
        if((feof(inputfile) != 0) && ((frames) < total_frames))
        {
          printf("\nError! Incomplete Audio frame: %i is %i values long.", frames, actual_fragment_size);
          break;
        }
      s_char = getc(inputfile);
      }
      if((actual_fragment_size != expected_fragment_size) && (feof(inputfile) == 0))
      {
        printf("\nError! Audio Frame %i is %i values long but %i values were expected.", frames, actual_fragment_size, 
expected_fragment_size);
      }
    }
    
    fread(&expected_frame_length, sizeof(expected_frame_length), 1, inputfile);
    if (feof(inputfile) != 0)
      break;
    s_char = getc(inputfile);
    while((s_char != MVPD_END_OF_PICTURE_FRAME) && (feof(inputfile) == 0))
      {
        length++;
	if((feof(inputfile) != 0) && ((frames) < total_frames))
	{
	  printf("\nError! Incomplete frame: %i is %i values long.", frames, length);
	  break;
	}


        if (s_char == MVPD_END_OF_PICTURE_BLOCK)
        {
          blocks++;
          if(detailed)
          {
            printf(".");
            fflush(stdout);
          }
        }
        s_char = getc(inputfile);
      }

      if(blocks != expected_blocks)
      {
        printf("\nError! Frame: %i has %i blocks.", frames, blocks);
      }

      if(length != expected_frame_length)
      {
	printf("\nError! Frame %i is %i values long but %i values were expected.", frames, length, expected_frame_length);
      }

      total_length += length;
      frames++;

      if(detailed)
      {
        printf("\nDurchlauf: %i  length: %i  blocks: %i", frames, length, blocks);
        fflush(stdout);
      }

      if(length > max_length)
        max_length = length;

      if((length < min_length) && (length > 3))
        min_length = length;

  }
  printf("\n");
  printf("\nFilename:\t\t%s", argv[1]);
  printf("\nFile version:\t\t%i", (int) file_version);
  printf("\nQuality setting:\t%i", (int) quality);
  printf("\nColor:\t\t\t%i", color);
  printf("\nStart offset:\t\t%i", start_offset);
  printf("\nSize: \t\t\tX: %i  Y: %i", x, y);
  printf("\nFrames expected:\t%i", total_frames);
  printf("\nActual frames:\t\t%i", frames);
  printf("\nMilisecs per frame:\t%i", (int) ms_per_frame);
  printf("\nShortest frame:\t\t%i", min_length);
  printf("\nLongest frame:\t\t%i", max_length);
  printf("\nExpected length:\t%i", expected_max_frame_length);
  printf("\nAverage:\t\t%i", (total_length / frames));
  printf("\n-------------------");
  printf("\nAudio Stuff:");
  printf("\nAudio?\t\t\t%i", audio);
  printf("\nFragment size:\t\t%i", expected_fragment_size);
  printf("\nBitrate:\t\t%i", bitrate);
  printf("\nChannels:\t\t%i", (stereo + 1));
  printf("\nSampling rate:\t\t%i", sample_rate);
  printf("\n\n");
  return 0;
}
