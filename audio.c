#include <stdio.h>
#include <stdlib.h>

#ifdef SOUND
#include <fcntl.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "mvpd.h"

static mvpd_int32 VOLUME = 32;

#ifdef SOUND
static mvpd_int8 audio_fd;
/* This function initialises the audio output device it returns -1 on failure */
mvpd_int8 MVPD_INIT_AUDIO(mvpd_uint16 fragment_size, mvpd_uint8 bitrate, mvpd_uint8 channels, mvpd_uint16 sample_rate)
{

  if ((audio_fd = open("/dev/dsp", O_WRONLY, 0)) == -1)
  {
    printf("\nError: Couldn't open audio device!.\n");
    return -1;
  }
  
  int format;
  if(bitrate == 8)
    format = AFMT_U8;
  else
    if(bitrate == 16)
      format = AFMT_S16_LE;
    else
    {
      printf("\nError: Unsupported bitrate: %i!\n", bitrate);
      return -1;
    }
  if(ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format) ==-1)
  {
    printf("\nError: Couldn't set audio format.\n");
    return -1;
  }
  if(format != AFMT_S16_LE)
  {
    printf("\nError: Audio format changed by driver.\n");
    return -1;
  }

  int stereo = channels -1;
  if(ioctl(audio_fd, SNDCTL_DSP_STEREO, &stereo) ==-1)
  {
    printf("\nError: Couldn't set stereo mode.\n");
    return -1;
  }
  if(stereo != (channels - 1))
  {
    printf("\nError: Stereo mode changed by driver.\n");
    return -1;
  }

  int speed = sample_rate;
  if(ioctl(audio_fd, SNDCTL_DSP_SPEED, &speed) ==-1)
  {
    printf("\nError: Couldn't set sampling rate.\n");
    return -1;
  }
  if(abs(speed - (sample_rate/1)) >= 100)
  {
    printf("\nError: Set sampling rate differs too much from desired sampling rate.\n");
    return -1;
  }

  mvpd_uint32 arg = 20480 + 0x00020000;
  int frag_size = 0;

  if(ioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &arg))
  {
    printf("\nError: Couldn't set fragment size!\n");
    return -1;
  }
  if(ioctl(audio_fd, SNDCTL_DSP_GETBLKSIZE, &frag_size) == -1)
  {
    printf("\nError: Couldn't querry fragment size!\n");
    return -1;
  }
  if(frag_size != 20480)
  {
    printf("\nError: Fragment size changed by driver: requested: %i  actual: %i!\n", fragment_size, frag_size);
  }
  
#ifdef DEBUG
  fprintf(debug_log,"\nVolume: %i\n", VOLUME);
  fprintf(debug_log,"\nFragment size: %i\n", frag_size);
  fflush(debug_log);
#endif
  return audio_fd;
}

void MVPD_STOP_AUDIO()
{
#ifdef DEBUG
  fprintf(debug_log,"\nClosing audio Device\n");
  fflush(debug_log);
#endif
  close(audio_fd);
}

#endif

void MVPD_INCREASE_VOLUME()
{
  VOLUME++;
  if(VOLUME > 255)
    VOLUME = 255;
}

void MVPD_DECREASE_VOLUME()
{
  VOLUME--;
  if(VOLUME < 1)
    VOLUME = 1;
}

mvpd_int8 MVPD_OPEN_WAVE(char* name, WAVE* wave)
{
  char text_buffer[4];
  unsigned char u_char;

  fflush(stdout);

  wave->filename = name;
  wave->wav_file = fopen(name, "rb");

  if (wave->wav_file == NULL)
  {
    printf("\nSorry, couldn't open file %s\n", name);
    return -1;
  }

  if(fread(text_buffer, sizeof(char), 4, wave->wav_file) != 4)
  {
    printf("\nThe file %s is not a valid wav file!\n", name);
    return -1;
  }
  if((text_buffer[0] != 'R') ||(text_buffer[1] != 'I') ||(text_buffer[2] != 'F') || (text_buffer[3] != 'F'))
  {
    printf("\nError: Wrong Header Data in file: %s\n", name);
    return -1;
  }

  fseek(wave->wav_file, 4, SEEK_CUR);

  if(fread(text_buffer, sizeof(char), 4, wave->wav_file) != 4)
  {
    printf("\nThe file %s is not a valid wav file!\n", name);
    return -1;
  }
  if((text_buffer[0] != 'W') ||(text_buffer[1] != 'A') ||(text_buffer[2] != 'V') || (text_buffer[3] != 'E'))
  {
    printf("\nError: Wrong Header Data in file: %s\n", name);
    return -1;
  }

  if(fread(text_buffer, sizeof(char), 4, wave->wav_file) != 4)
  {
    printf("\nThe file %s is not a valid wav file!\n", name);
    return -1;
  }
  if((text_buffer[0] != 'f') ||(text_buffer[1] != 'm') ||(text_buffer[2] != 't'))
  {
    printf("\nError: Wrong Header Data in file: %s\n", name);
    return -1;
  }

  if((u_char = fgetc(wave->wav_file)) != 16)
  {
    printf("\nThe file %s is not a valid wav file!\n", name);
    return -1;
  }

  fseek(wave->wav_file, 5, SEEK_CUR);
  fread(&wave->channels, sizeof(char), 2, wave->wav_file);
  fread(&wave->sample_rate, sizeof(char), 4, wave->wav_file);
  fread(&wave->bytes_per_sec, sizeof(char), 4, wave->wav_file);
  fread(&wave->block_align, sizeof(char), 2, wave->wav_file);
  fread(&wave->bits_per_sample, sizeof(char), 2, wave->wav_file);

  fread(text_buffer, sizeof(char), 4, wave->wav_file);
  int gefunden = 0;
  while(gefunden == 0)
  {

    if((text_buffer[0] == 'd') && (text_buffer[1] == 'a') && (text_buffer[2] == 't') && (text_buffer[3] == 'a'))
    {
      gefunden =1;
    }
    else
    {
      text_buffer[0] = text_buffer[1];
      text_buffer[1] = text_buffer[2];
      text_buffer[2] = text_buffer[3];
      text_buffer[3] = fgetc(wave->wav_file);
    }
  }
  fread(&wave->size_audio_data, sizeof(mvpd_uint32), 1, wave->wav_file);
  wave->position = 0;
  wave->fragment_size=0;

#ifdef MVPD_DEBUG
  printf("\nWav-Header:");
  printf("\nFilename:     %s", wave->filename);
  printf("\nKanaele:      %i", wave->channels);
  printf("\nSample rate:  %i", wave->sample_rate);
  printf("\nBytes / sec:  %i", wave->bytes_per_sec);
  printf("\nBlock align:  %i", wave->block_align);
  printf("\nBit rate:     %i", wave->bits_per_sample);
  printf("\nLaenge:       %i", wave->size_audio_data);
  printf("\nPosition:     %i", wave->position);
  printf("\nFragmentsize: %i", wave->fragment_size);
  printf("\n");
#endif

  if(wave->channels > 3)
{
  printf("\nSorry only mono or stereo wav's are supported right now");
  return -1;
}
  if(wave->bits_per_sample != 16)
{
  printf("\nSorry only 16 Bit wav's are supported right now");
  return -1;
}



  return 0;
}

void MVPD_CLOSE_WAVE(WAVE* wave)
{
  fclose(wave->wav_file);
}


mvpd_uint32 MVPD_INIT_WAVE(WAVE* wave, mvpd_uint32 fragment_size)
{
  mvpd_uint32 sample_size = (wave->channels * wave->bits_per_sample) / 8;
  if((fragment_size % sample_size) != 0)
  {
    printf("\nError: The fragment size must be a multiple of the sample size!\n");
    return -1;
  }

  mvpd_uint32 number_of_fragments =(wave->size_audio_data / fragment_size);
  wave->fragment_size = fragment_size;
  return number_of_fragments;
}


mvpd_uint32 MVPD_LOAD_WAVE_DATA(mvpd_int16* audio_buffer, mvpd_uint32 buffer_size, WAVE* wave)
{
  mvpd_uint32 remaining_bytes = (wave->size_audio_data - wave->position);
  mvpd_uint32 padding_bytes = 0;
  
  mvpd_uint32 i;
  
  if(remaining_bytes < buffer_size)
  {
    printf("\nOutput will be padded");
    padding_bytes = buffer_size - remaining_bytes;
    buffer_size = remaining_bytes;
  }

  fread(audio_buffer, sizeof(char), buffer_size, wave->wav_file);

  for(i = 0; i < (padding_bytes/2); i++)
    audio_buffer[i] = -32768;
  
  wave->position += buffer_size;
  return buffer_size;
}

mvpd_int8 MVPD_COMPRESS_AUDIO(mvpd_int16* input, mvpd_uint32 length, mvpd_int8* output)
{
  mvpd_uint32 i;
  for(i = 0; i <length; i++)
  {
    output[i] = ((input[i] >> 8));
    if(output[i] > 125)
      output[i] = 125;
    if(output[i] < -125)
      output[i] = -125;
  }
  return 0;
}

mvpd_int8 MVPD_DECOMPRESS_AUDIO(mvpd_int8* input, mvpd_uint32 length, mvpd_int16* output)
{
  mvpd_uint32 i;
  for(i = 0; i <length; i++)
    output[i] = (input[i]) << 8;
  return 0;
}

mvpd_int8 MVPD_DECOMPRESS_AUDIO2(mvpd_int8* input, mvpd_uint32 length, mvpd_int16* output)
{
  mvpd_uint32 i;
  for(i = 0; i <length; i++)
    output[i] = ((mvpd_int32) ( (mvpd_int32) (input[i]) * ((VOLUME << 8) / 32)) & 0x8000FFFF);
  return 0;
}

mvpd_int8 MVPD_INFLATE_AUDIO(mvpd_int16* input, mvpd_uint32 length)
{
  mvpd_uint32 i;
  for(i = length-1; i > 0; i--)
  {
    input[2 * i] = input[i];
    input[(2 * i) +1] = input[i];
  }
  return 0;
}
