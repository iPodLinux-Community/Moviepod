#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "mvpd.h"
#include "icons.h"

#ifdef IPOD
#include "../includes/hotdog.h"
#else
#include "SDL.h"
#endif

mvpd_uint8 QUALITY = 11;
mvpd_uint8 COLOR = 1;
mvpd_uint8 MS_PER_FRAME = 0;
mvpd_uint16 MATRIX_X = 0;
mvpd_uint16 MATRIX_Y = 0;

mvpd_uint16 SCREEN_X = 220;
mvpd_uint16 SCREEN_Y = 176;

FILE* inputfile = NULL;
  
//Video Arrays:
  mvpd_int8* LIST = NULL;
  mvpd_uint8* bild = NULL;
  mvpd_uint8* DCT_Y = NULL;
  mvpd_uint8* DCT_U = NULL;
  mvpd_uint8* DCT_V = NULL;
  mvpd_uint16* screen = NULL;
  mvpd_uint8* screen_buffer = NULL;
//Audio Arrays:
  mvpd_int16* audio_buffer = NULL;
  mvpd_int8* compressed_audio = NULL;

  mvpd_uint32 start_offset = 0;
  mvpd_uint32 audio_offset = 0;


mvpd_int32 buffer_audio(mvpd_int32 position, mvpd_uint32* frame_index, mvpd_uint16 fragment_size, mvpd_int8 audio_dev);

void terminate()
{
#ifdef DEBUG
  fprintf(debug_log,"\n\nEntered Quit function: ");
  fflush(debug_log);
#endif

#ifdef IPOD
  HD_LCD_Quit();
  MVPD_STOP_INPUT();
#ifdef SOUND
  MVPD_STOP_AUDIO();
#endif
#endif

  MVPD_CLOSE_DECODING();

  if(LIST != NULL)
    free(LIST);
  LIST = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed LIST.");
  fflush(debug_log);
  printf("\nFreeed LIST.");
  fflush(stdout);
#endif
  
  if(bild != NULL)
    free(bild);
  bild = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed bild.");
  fflush(debug_log);
  printf("\nFreeed bild.");
  fflush(stdout);
#endif
  
  if(DCT_Y != NULL)
    free(DCT_Y);
  DCT_Y = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed DCT_Y.");
  fflush(debug_log);
  printf("\nFreeed DCT_Y.");
  fflush(stdout);
#endif
  
  if(DCT_U != NULL)
    free(DCT_U);
  DCT_U = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed DCT_U.");
  fflush(debug_log);
  printf("\nDCT_U.");
  fflush(stdout);
#endif
  
  if(DCT_V != NULL)
    free(DCT_V);
  DCT_V = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed DCT_V.");
  fflush(debug_log);
  printf("\nDCT_V.");
  fflush(stdout);
#endif
  
  if(screen != NULL)
    free(screen);
  screen = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed screen.");
  fflush(debug_log);
  printf("\nFreeed screen.");
  fflush(stdout);
#endif
  
  if(screen_buffer != NULL)
    free(screen_buffer);
  screen_buffer  = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed screen_buffer.");
  fflush(debug_log);
  printf("\nFreeed screen_buffer.");
  fflush(stdout);
#endif

  if(audio_buffer != NULL)
    free(audio_buffer);
  audio_buffer = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed audio_buffer.");
  fflush(debug_log);
  printf("\nFreeed audio_buffer.");
  fflush(stdout);
#endif

  if(compressed_audio != NULL)
    free(compressed_audio);
  compressed_audio = NULL;
#ifdef DEBUG
  fprintf(debug_log,"\nFreeed compressed_audio.");
  fflush(debug_log);
  printf("\nFreeed compressed audio.");
  fflush(stdout);
#endif

  if(inputfile != NULL)
    fclose(inputfile);
#ifdef DEBUG
  fprintf(debug_log,"\nInput file closed.");
  fflush(debug_log);
  printf("\nInput file closed.");
  fflush(stdout);
#endif

#ifdef DEBUG
  fprintf(debug_log,"\nClosing log.");
  fclose(debug_log);
  printf("\nLog closed.");
  fflush(stdout);
#endif
  printf("\n\n");
  
}


#ifndef IPOD
SDL_Surface *sdl_screen;
void putpixel(int x, int y, int color)
{
  unsigned int *ptr = (unsigned int*)sdl_screen->pixels;
  int lineoffset = y * (sdl_screen->pitch / 4);
  ptr[lineoffset + x] = color;
}

void render(mvpd_uint8 bild[])
{
  mvpd_uint32 farbe;

  // Lock surface if needed
  if (SDL_MUSTLOCK(sdl_screen))
    if (SDL_LockSurface(sdl_screen) < 0)
      return;

  mvpd_int32 x,y;
  for(y = 0; y < SCREEN_Y; y++)
    for(x = 0; x < SCREEN_X; x++)
  {
    farbe = bild[(((y * SCREEN_X) + x) * 3) + 0];
    farbe = bild[(((y * SCREEN_X) + x) * 3) + 1] + farbe * 256;
    farbe = bild[(((y * SCREEN_X) + x) * 3) + 2] + farbe * 256;

    putpixel(x,y, farbe);
  }

  // Unlock if needed
  if (SDL_MUSTLOCK(sdl_screen))
    SDL_UnlockSurface(sdl_screen);

  // Tell SDL to update the whole screen
  SDL_UpdateRect(sdl_screen, 0, 0, SCREEN_X, SCREEN_Y);

}

void SDL_START()
{
  // Initialize SDL's subsystems - in this case, only video.
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
#ifdef DEBUG
    printf("\nSDL initialisiert.");
#endif

  // Register SDL_Quit to be called at exit; makes sure things are
  // cleaned up when we quit.
  atexit(SDL_Quit);
  sdl_screen = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, 32, SDL_SWSURFACE);

#ifdef DEBUG
    printf("Screen initialisert: ");
#endif

  // If we fail, return error.
  if ( sdl_screen == NULL )
  {
    fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
    exit(1);
  }
}
#endif


int main(int argc, char *argv[])
{
  mvpd_uint32 start_time, current_time, temp_time = 0;
  mvpd_uint32 time_elapsed = 0;
  mvpd_uint32 time_to_wait = 0;
  mvpd_uint32 lag = 0;
  printf("\nWelcome to MoviePod!\n");
#ifdef DEBUG
    setbuf(stdout,0);
    debug_log = fopen("/home/mvpd_pre_hd.log","w");
    if(debug_log == NULL)
    {
      printf("\nError: Couldn't create log file!\n");
      return -1;
    }
    fprintf(debug_log,"Movie Player started.\n");
    fflush(debug_log);
#endif

  atexit(terminate);

if(argc > 1)
  inputfile = fopen(argv[1], "rb");
else
  inputfile = fopen("nano.mvpd", "rb");
  
  if (inputfile == NULL)
  {
    printf("Sorry the file %s could not be openend.\nThe program will be terminated", argv[1]);
    return -1;
  }

  unsigned char u_char;
  mvpd_int16 key;
  mvpd_uint8 header_buffer[4];
  mvpd_uint8 file_version = 0;
  mvpd_uint8 ms_per_frame = 0;
  mvpd_uint16 expected_max_frame_length = 0;

  mvpd_uint32 index = 0;
  mvpd_uint32 total_frames=0;
  bool clip = false;

    //Audio variables:
  mvpd_uint8 audio = 0;
  mvpd_uint16 fragment_size = 0;
  mvpd_uint8 bitrate = 0;
  mvpd_uint8 stereo = 0;
  mvpd_uint16 sample_rate = 0;
  mvpd_int8 audio_dev = 0;

  mvpd_uint32* frame_index = NULL;



  fread(header_buffer, 1, 4, inputfile);
  if((header_buffer[0] == 'M') && (header_buffer[0] == 'V') && (header_buffer[0] == 'P') && (header_buffer[0] == 'D'))
  {
    printf("\nHeader error, first bytes should be MVPD, but are %s.", header_buffer);
    return -1;
  }

  file_version = getc(inputfile);
  printf("\nFile Version: %i", file_version);

  if(file_version < 4)
  {
    mvpd_uint16 temp_offset = 0;
    fread(&temp_offset, sizeof(temp_offset), 1, inputfile);
    start_offset = temp_offset;
  }
  else
    fread(&start_offset, sizeof(start_offset), 1, inputfile);
  
  
  
  fread(&MATRIX_X, sizeof(MATRIX_X), 1, inputfile);
  fread(&MATRIX_Y, sizeof(MATRIX_Y), 1, inputfile);
  if(file_version > 1)
    fread(&expected_max_frame_length, sizeof(expected_max_frame_length), 1, inputfile);
  else
    expected_max_frame_length = MATRIX_X * MATRIX_Y * 2;
#ifdef DEBUG
    fprintf(debug_log,"\nLongest frame: %i", expected_max_frame_length);
    fflush(debug_log);
    printf("\nLongest frame: %i", expected_max_frame_length);
    fflush(stdout);
#endif


  QUALITY = getc(inputfile);
  u_char = getc(inputfile);
  if(u_char == 'c')
    COLOR = 1;
  else if(u_char == 'b')
    COLOR = 0;
  else
  {
    printf("\nError wrong color indicator!");
    return -1;
  }

  fread(&total_frames, sizeof(total_frames), 1, inputfile);

#ifdef DEBUG
    fprintf(debug_log,"\nTotal number of frames: %i", total_frames);
    fflush(debug_log);
    printf("\nTotal number of frames: %i", total_frames);
    fflush(stdout);
#endif

  ms_per_frame = getc(inputfile);

  //Version 0 didn't have sound support
  if(file_version > 0)
  {
    fread(&audio, sizeof(audio), 1, inputfile);
    if(audio)
    {
      fread(&fragment_size, sizeof(fragment_size), 1, inputfile);
      fread(&bitrate, sizeof(bitrate), 1, inputfile);
      fread(&stereo, sizeof(stereo), 1, inputfile);
      fread(&sample_rate, sizeof(sample_rate), 1, inputfile);
#ifdef SOUND
#ifdef DEBUG
      fprintf(debug_log,"\nAudio data expected.");
      fflush(debug_log);
      printf("\nAudio data expected.");
      fflush(stdout);
#endif
      audio_buffer = (mvpd_int16*) calloc(10240, sizeof(mvpd_int16));
      if(audio_buffer == NULL)
      {
        printf("\nError: Couldn't allocate memory for audio_buffer\n");
#ifdef DEBUG
        fprintf(debug_log,"\nError: Couldn't allocate memory for audio_buffer\n");
#endif
        return -1;
      }
#ifdef DEBUG
      fprintf(debug_log,"\nAudio buffer initialized.");
      fflush(debug_log);
      printf("\nAudio buffer initialized.");
      fflush(stdout);
#endif
      compressed_audio = (mvpd_int8*) calloc((10240 + 1), sizeof(mvpd_int8));
      if(compressed_audio == NULL)
      {
        printf("\nError: Couldn't allocate memory for compressed_audio buffer\n");
#ifdef DEBUG
        fprintf(debug_log,"\nError: Couldn't allocate memory for compressed_audio buffer\n");
        fflush(debug_log);
#endif
        return -1;
      }
#ifdef DEBUG
      fprintf(debug_log,"\ncompressed_audio initialized.");
      fflush(debug_log);
#endif
      audio_dev =  MVPD_INIT_AUDIO(fragment_size, 16, stereo + 1, sample_rate);
#endif
      if(audio_dev == -1)
      {
        printf("\nError while initiliasing audio device!");
#ifdef DEBUG
        fprintf(debug_log,"\nError while initiliazing audio device\n");
        fflush(debug_log);
#endif
        return -1;
      }
    }
  }

  
  if(file_version > 3)
  {
    //Set up Index table
    frame_index = (mvpd_uint32*) calloc((total_frames + 2), sizeof(mvpd_int32));
    if(frame_index == NULL)
    {
      printf("\nError: Couldn't allocate memory for frame_index\n");
#ifdef DEBUG
      fprintf(debug_log,"\nError: Couldn't allocate memory for frame_index\n");
      fflush(debug_log);
#endif
      return -1;
    }
    frame_index[0] = total_frames;
    if(total_frames != fread(&frame_index[1], sizeof(mvpd_int32), total_frames, inputfile))
    {
      printf("\nError: Couldn't read frame_index\n");
#ifdef DEBUG
      fprintf(debug_log,"\nError: Couldn't read frame_index\n");
      fflush(debug_log);
#endif
      return -1;
    }
#ifdef DEBUG
      fprintf(debug_log,"\nNumber of frames: %i", frame_index[0]);
      fprintf(debug_log,"\nFirst frame:      %i", frame_index[1]);
      fprintf(debug_log,"\nSecond frame:     %i", frame_index[2]);
      fflush(debug_log);
#endif
  }
  
  
  
#ifdef DEBUG
  fprintf(debug_log,"\nSize: X: \t%i  Y: %i", MATRIX_X, MATRIX_Y);
  fprintf(debug_log,"\nStartoffset: %i", start_offset);
  fflush(debug_log);
  printf("\nSize: X: \t%i  Y: %i", MATRIX_X, MATRIX_Y);
  printf("\nStartoffset: %i", start_offset);
  fflush(stdout);
#endif

//Initiliaze the different Video Buffers. Unfortunately this is rather cluttered by the debug statements
  LIST = (mvpd_int8*) calloc(expected_max_frame_length + 1, sizeof(mvpd_int8));
  if(LIST == NULL)
  {
    printf("\nError: Couldn't allocate memory for video list buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for video list buffer\n");
    fflush(debug_log);
#endif
    return -1;
  }
#ifdef DEBUG
  fprintf(debug_log,"\nLIST initialized.");
  fflush(debug_log);
  printf("\nLIST initialized.");
  fflush(stdout);
#endif
  bild = (mvpd_uint8*) calloc((MATRIX_Y * MATRIX_X * 3), sizeof(mvpd_uint8));
  if(bild == NULL)
  {
    printf("\nError: Couldn't allocate memory for picture buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for picture buffer\n");
    fflush(debug_log);
#endif
    return -1;
  }
#ifdef DEBUG
  fprintf(debug_log,"\nbild initialized.");
  fflush(debug_log);
  printf("\nbild initialized.");
  fflush(stdout);
#endif
  DCT_Y = (mvpd_uint8*) calloc((MATRIX_Y * MATRIX_X), sizeof(mvpd_uint8));
  DCT_U = (mvpd_uint8*) calloc(((MATRIX_Y/2) * (MATRIX_X/2)), sizeof(mvpd_uint8));
  DCT_V = (mvpd_uint8*) calloc(((MATRIX_Y/2) * (MATRIX_X/2)), sizeof(mvpd_uint8));
  if((DCT_Y == NULL) || (DCT_U == NULL) || (DCT_V == NULL))
  {
    printf("\nError: Couldn't allocate memory for DCT buffers\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for DCT buffers\n");
    fflush(debug_log);
#endif
    return -1;
  }
#ifdef DEBUG
  fprintf(debug_log,"\nDCT buffers initialized.");
  fflush(debug_log);
  printf("\nDCT BUFFERS initialized.");
  fflush(stdout);
#endif

  mvpd_uint32 position;
  mvpd_uint32 i = 0;

  mvpd_uint8 dct_temp = 127;
  if(COLOR)
    dct_temp = 0;
  
  for(i = 0; i < (MATRIX_X * MATRIX_Y); i++)
    DCT_Y[i] = 0;
  
  for(i = 0; i < (MATRIX_X * MATRIX_Y)/4; i++)
  {   
    DCT_U[i] = dct_temp;
    DCT_V[i] = dct_temp;
  }
  mvpd_uint16 frame_length;

  MVPD_INITIALIZE_QUANT(QUALITY);

#ifdef DEBUG
    fclose(debug_log);
    debug_log = fopen("/home/mvpd_post_hd.log","w");
    if(debug_log == NULL)
    {
      printf("\nError: Couldn't create log file!\n");
      return -1;
    }
#endif

#ifdef IPOD
#ifdef DEBUG
  fprintf(debug_log,"\nInitializing Hotdog: ");
  fflush(debug_log);
  printf("\nInitializing Hotdog: \n");
  fflush(stdout);
#endif
  MVPD_START_INPUT();
  HD_LCD_Init();
#ifdef DEBUG
  fprintf(debug_log,"Hotdog initialized.");
  fflush(debug_log);
#endif

  int hw_ver;
  int lcd_width;
  int lcd_height;
  int lcd_type;
#ifdef DEBUG
  fprintf(debug_log,"\nRequesting LCD info: ");
  fflush(debug_log);
#endif

  HD_LCD_GetInfo (&hw_ver, &lcd_width, &lcd_height, &lcd_type);
#ifdef DEBUG
  fprintf(debug_log,"\nLCD Version: %i", hw_ver);
  fprintf(debug_log,"\nLCD Width:   %i", lcd_width);
  fprintf(debug_log,"\nLCD Height:  %i", lcd_height);
  fprintf(debug_log,"\nLCD Type:    %i", lcd_type);
  fflush(debug_log);
#endif
  
  SCREEN_X = lcd_width;
  SCREEN_Y = lcd_height;
  
  screen = (mvpd_uint16*) calloc((SCREEN_X * SCREEN_Y), sizeof(mvpd_uint16)) ;
  if(screen == NULL)
  {
    printf("\nError: Couldn't allocate memory for screen buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for screen buffer\n");
#endif
    return -1;
  }
#ifdef DEBUG
  fprintf(debug_log,"\nscreen initialized.");
#endif
  for(i = 0; i < (SCREEN_X * SCREEN_Y); i++)
    screen[i] = 0xFFFF;
#else
//  SCREEN_X = MATRIX_X;
//  SCREEN_Y = MATRIX_Y;
#ifdef DEBUG
  fprintf(debug_log,"\nStarting SDL.");
  fflush(debug_log);
#endif
  
  SDL_START();
#endif

  if((SCREEN_X != MATRIX_X) || (MATRIX_Y != SCREEN_Y))
    clip = true;
#ifdef DEBUG
    fprintf(debug_log,"\nClipping: %i", clip);
    fflush(debug_log);
#endif
  
  screen_buffer = (mvpd_uint8*) calloc((SCREEN_Y * SCREEN_X * 3), sizeof(mvpd_uint8));
  if(screen_buffer == NULL)
  {
    printf("\nError: Couldn't allocate memory for screen_buffer buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for screen_buffer buffer\n");
#endif
    return -1;
  }
#ifdef DEBUG
  fprintf(debug_log,"\nscreen_buffer initialized.");
#endif
  for(i = 0; i < (SCREEN_X * SCREEN_Y); i++)
  {
    screen_buffer[3 * i + 0] = 0;
    screen_buffer[3 * i + 1] = 127;
    screen_buffer[3 * i + 2] = 127;
  }

  MVPD_INIT_DECODING();

  i = 0;
  bool play = true;
  bool pause_first = true;
  bool fast_forward = false;
  bool rewind = false;
  mvpd_uint16 forward_intervall = 50;
  mvpd_uint32 frame_counter = 0;
  bool play_sound = true;

#ifdef DEBUG
  fprintf(debug_log,"\nStarting Video Playback.\n");
  fflush(debug_log);
#endif
#ifdef DEBUG
    fclose(debug_log);
    debug_log = fopen("/home/mvpd_play.log","w");
    if(debug_log == NULL)
    {
      printf("\nError: Couldn't create log file!\n");
      return -1;
    }
    fprintf(debug_log,"Movie Player started.\n");
    fflush(debug_log);
#endif
  
#ifdef IPOD
  start_time = inl(0x60005010);
#ifdef DEBUG
    fprintf(debug_log,"Start Time: %i\n", start_time);
    fflush(debug_log);
#endif
#endif
  
  
#ifdef SOUND
  if(file_version > 3)
  {
    //Audio Stuff:
    if(audio)
    {
      audio_offset = ftell( inputfile );
#ifdef DEBUG
      fprintf(debug_log,"\nAudio offset: %i", audio_offset);
      fprintf(debug_log,"\nReading Audio Data: ");
      fflush(debug_log);
#endif
      if(10240 != (fread(compressed_audio, sizeof(mvpd_int8), 10240, inputfile)))
        return -1;
      MVPD_DECOMPRESS_AUDIO2(compressed_audio, 10240, audio_buffer);
      if((write(audio_dev, audio_buffer, (2 * 10240))) != (2 * 10240))
        return -1;
      if((10240 - fragment_size) != (fread(compressed_audio, sizeof(mvpd_int8), (10240 - fragment_size), inputfile)))
        return -1;
      MVPD_DECOMPRESS_AUDIO2(compressed_audio, (10240 - fragment_size), audio_buffer);
      if((write(audio_dev, audio_buffer, (2 * (10240 - fragment_size)))) != (2 * (10240 - fragment_size)))
        return -1;
    }
  }
#endif
  
  fseek(inputfile, start_offset, SEEK_SET);
  
  
  while(i < total_frames)
  {
    frame_counter++;
    if(rewind)
      if (file_version > 3)
        if(i > forward_intervall)
          fseek(inputfile, frame_index[i-=forward_intervall], SEEK_SET);

    if(fast_forward)
      if (file_version > 3)
        if((i + forward_intervall + 5) < frame_index[0])
          fseek(inputfile, frame_index[i+=forward_intervall], SEEK_SET);
    
    if ((feof(inputfile) == 0) && (play || pause_first))
    {
#ifdef DEBUG
  fprintf(debug_log,"\n\nFrame: %i", i);
  if(rewind)
    fprintf(debug_log,"\n--Rewinding--");
  if(fast_forward)
    fprintf(debug_log,"\n--Fast Forwarding--");
  fflush(debug_log);
#endif

      //Audio Stuff:
      if(audio)
      {
#ifdef SOUND
#ifdef DEBUG
        fprintf(debug_log,"\nReading Audio Data: ");
        fflush(debug_log);
#ifdef IPOD
  temp_time = inl(0x60005010);
#endif
#endif
        if((fragment_size + 1) != (fread(compressed_audio, sizeof(mvpd_int8), fragment_size+1, inputfile)))
          return -1;

        if(compressed_audio[fragment_size] != MVPD_END_OF_AUDIO_FRAME)
        {
          printf("\nError while reading audio data!");
#ifdef DEBUG
          fprintf(debug_log,"\nError while reading audio data...\n");
#endif
          return -1;
        }

#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
        fprintf(debug_log,"\nDecompressing Audio Data: ");
        fflush(debug_log);
#endif
        if((play_sound) && (fast_forward == false) &&  (rewind == false))
        {
          MVPD_DECOMPRESS_AUDIO2(compressed_audio, fragment_size, audio_buffer);
          if((write(audio_dev, audio_buffer, (2 * fragment_size))) != (2 * fragment_size))
            return -1;
        }
#else
        fseek(inputfile, fragment_size+1, SEEK_CUR);

#endif
      }

#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
      fprintf(debug_log,"\nReading Video Data:");
      fflush(debug_log);
#endif
      index=0;
      fread(&frame_length, sizeof(frame_length), 1, inputfile);
#ifdef DEBUG
        fprintf(debug_log,"\n   length: %i bytes\n", frame_length);
        fflush(debug_log);
#endif

      if((frame_length + 1) != (fread(LIST, sizeof(mvpd_int8), frame_length+1, inputfile)))
      {
        printf("\nError while reading video data!");
#ifdef DEBUG
        fprintf(debug_log,"\nError while reading video data...\n");
#endif
        return -1;
      }

#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"   time: %i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
      fprintf(debug_log,"\nDecompressing Video Data.\n   Zigzag: ");
      fflush(debug_log);
#endif
//      if(forward_intervall < 600)
//        forward_intervall += 1;

      if(/*((fast_forward == false)  &&  (rewind == false)) || ((frame_counter % forward_intervall) == 0)*/ true)
      {

        position = MVPD_IZIGZAG(LIST, (position = 0), LUMINANCE, DCT_Y, MATRIX_X, MATRIX_Y);
        if(COLOR)
        {
          position = MVPD_IZIGZAG(LIST, position, CHROMINANCE, DCT_U, (MATRIX_X / 2), (MATRIX_Y / 2));
          position = MVPD_IZIGZAG(LIST, position, CHROMINANCE, DCT_V, (MATRIX_X / 2), (MATRIX_Y / 2));
        }
#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
      fprintf(debug_log,"\n   Chroma Subsampling: ");
      fflush(debug_log);
#endif
        
        REVERSE_CHROMA_SUBSAMPLING(DCT_Y, DCT_U, DCT_V, bild, MATRIX_X, MATRIX_Y);
          
#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
      fprintf(debug_log,"\nCliping: ");
      fflush(debug_log);
#endif
        
        if(play && (fast_forward == 0)  &&  (rewind == false) && clip)
          MVPD_CLIP(blank_icon, (6 * 3), 6, screen_buffer, (3 * SCREEN_X), 12);
        
        if(clip)
          MVPD_CLIP(bild, (3 * MATRIX_X), MATRIX_Y, screen_buffer, (3 * SCREEN_X), SCREEN_Y);
          
        if(fast_forward)
        {
          if(clip)
            MVPD_CLIP(fforward_icon, (6 * 3), 6, screen_buffer, (3 * SCREEN_X), 12);
          else
            MVPD_CLIP(fforward_icon, (6 * 3), 6, bild, (3 * SCREEN_X), 12);
        }
        
        if(rewind)
        {
          if(clip)
            MVPD_CLIP(rewind_icon, (6 * 3), 6, screen_buffer, (3 * SCREEN_X), 12);
          else
            MVPD_CLIP(rewind_icon, (6 * 3), 6, bild, (3 * SCREEN_X), 12);
        }
        
        if(play == 0)
        {
          if(clip)
            MVPD_CLIP(pause_icon, (6 * 3), 6, screen_buffer, (3 * SCREEN_X), 12);
          else
            MVPD_CLIP(pause_icon, (6 * 3), 6, bild, (3 * SCREEN_X), 12);
        }
        pause_first = 0;

#ifdef DEBUG
#ifdef IPOD
        fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
        temp_time = inl(0x60005010);
#endif
        fprintf(debug_log,"\nConverting to RGB: ");
        fflush(debug_log);
#endif
#ifdef IPOD
        if(clip)
          MVPD_YUV_ARRAY_TO_RGB565_ARRAY(screen_buffer, (SCREEN_X * SCREEN_Y), screen);
        else
          MVPD_YUV_ARRAY_TO_RGB565_ARRAY(bild, (SCREEN_X * SCREEN_Y), screen);
      
#else
        if(clip)
          MVPD_YUV_ARRAY_TO_RGB_ARRAY(screen_buffer, SCREEN_X, SCREEN_Y);
        else
          MVPD_YUV_ARRAY_TO_RGB_ARRAY(bild, SCREEN_X, SCREEN_Y);
      
#endif
      }
      i++;
    }

    // Render stuff
#ifdef DEBUG
    if(play)
    {
#ifdef IPOD
      fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
      temp_time = inl(0x60005010);
#endif
      fprintf(debug_log,"\nBlitting to screen: ");
      fflush(debug_log);
    }
#endif
#ifdef IPOD
    if(/*((fast_forward == 0) && (rewind == false)) || ((frame_counter % forward_intervall) == 0)*/ true)
      HD_LCD_Update (screen,0,0,SCREEN_X,SCREEN_Y);
    
#ifdef DEBUG
#ifdef IPOD
    if(play)
      fprintf(debug_log,"%i ms.", (int) ((inl(0x60005010) - temp_time) / 1000));
#endif
#endif
    
    //Check if a Key has been pressed:
    key = MVPD_GET_KEY();
    while(key != -1)
    {
      key = KEYCODE(key);
      switch(key)
      {
        case KH_KEY_MENU:
#ifdef DEBUG
          fprintf(debug_log,"\nExit signal recieved: \n");
          fflush(debug_log);
#endif
          play = 0;
          return 0;
          break;
        
        case KH_KEY_PLAY:
        case KH_KEY_HOLD:
          play = 1 - play;
          fast_forward = 0;
          rewind = 0;
          
          pause_first = 1;
          
          if (file_version > 3)
            if(play)
              buffer_audio(i, frame_index, fragment_size, audio_dev);

#ifdef DEBUG
          fprintf(debug_log,"\nPlay toggle: %i", play);
          fflush(debug_log);
#endif
          break;
        
        case KH_KEY_FORWARD:
#ifdef DEBUG
          fprintf(debug_log,"\nfast Forward: %i", fast_forward);
          fflush(debug_log);
#endif
          rewind = 0;
          fast_forward = 1 - fast_forward;
          if(fast_forward == 0)
            if (file_version > 3)
              buffer_audio(i, frame_index, fragment_size, audio_dev);
          break;
        
        
        case KH_KEY_REWIND:
#ifdef DEBUG
          fprintf(debug_log,"\nRewind: %i", fast_forward);
          fflush(debug_log);
#endif
          fast_forward = 0;
          rewind = 1 - rewind;
          if(rewind == 0)
            if (file_version > 3)
              buffer_audio(i, frame_index, fragment_size, audio_dev);
          break;
        
        case KH_KEY_ACTION:
          play_sound = 1 - play_sound;
          break;
          
        case KH_WHEEL_L:
          MVPD_DECREASE_VOLUME();
#ifdef DEBUG
          fprintf(debug_log,"\nLeiser");
          fflush(debug_log);
#endif
          break;
          
        case KH_WHEEL_R:
          MVPD_INCREASE_VOLUME();
#ifdef DEBUG
          fprintf(debug_log,"\nLauter");
          fflush(debug_log);
#endif
          break;
      
      }
      key = MVPD_GET_KEY();
    }

    current_time = inl(0x60005010);
    
    if(play)
    {
#ifdef DEBUG
      fprintf(debug_log,"\nStart Time:        %i", start_time);
      fprintf(debug_log,"\nCurrent Time:      %i", current_time);
      fprintf(debug_log,"\nCurrent lag:       %i ms", (lag / 1000));
#endif
    
      time_elapsed = (current_time - start_time);
      if((time_elapsed + lag) < ((1000 * ms_per_frame)))
      {
        time_to_wait = ((1000 * ms_per_frame)) - (time_elapsed + lag);
        lag = 0;
      }
      else
      {
        time_to_wait = 0;
        lag = (time_elapsed + lag) - (1000 * ms_per_frame);
      }
#ifdef DEBUG
      fprintf(debug_log,"\nTime elapsed:      %i ms", (time_elapsed / 1000));
      fprintf(debug_log,"\nDo i have to wait? %i", ((time_elapsed + lag) < ((1000 * ms_per_frame))));
      fprintf(debug_log,"\nTime to wait:      %i ms", (time_to_wait / 1000));
      fprintf(debug_log,"\nNew lag:           %i ms\n", (lag / 1000));
      fflush(debug_log);
#endif
    }
    
    start_time = inl(0x60005010);
    if(((fast_forward) == 0) && (play))
      while((start_time + time_to_wait) > (current_time))
      {
#ifdef DEBUG
        fprintf(debug_log,".");
        fflush(debug_log);
#endif
        current_time = inl(0x60005010);
      }
    start_time = inl(0x60005010);

#else
    if(clip)
      render(screen_buffer);
    else
      render(bild);
    
    // Poll for events, and handle the ones we care about.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
        case SDL_KEYDOWN:
          break;
        case SDL_KEYUP:
        // If escape is pressed, return (and thus, quit)
          if ((event.key.keysym.sym == SDLK_ESCAPE) || (event.key.keysym.sym == SDLK_q))
          {
            printf("\nExit signal recieved: ");
#ifdef DEBUG
            fprintf(debug_log,"\nExit signal recieved: \n");
            fflush(debug_log);
#endif
          return 0;
          }
          if (event.key.keysym.sym == SDLK_p)
          {
            play = 1 - play;
            pause_first = 1;
          }
          if (event.key.keysym.sym == SDLK_f)
          {
            forward_intervall = 10;
            fast_forward = 1 - fast_forward;
          }
          if (event.key.keysym.sym == SDLK_a)
          {
            play_sound = 1 - play_sound;
          }
          break;
        case SDL_QUIT:
          return(0);
      }
    }
#endif

  }
#ifdef DEBUG
#ifdef IPOD
  current_time = inl(0x60005010);
  fprintf(debug_log,"\nTotal runtime: %i ms\n", ((current_time -start_time) / 1000));
  fprintf(debug_log,"\nAll Good, closing debug_log.");
#endif
#endif
  return 0;
}


mvpd_int32 buffer_audio(mvpd_int32 position, mvpd_uint32* frame_index, mvpd_uint16 fragment_size, mvpd_int8 audio_dev)
{
  //How much Audio Data do I have to buffer?
  mvpd_uint16 load_size = 20480 - fragment_size;
  mvpd_uint16 fragments_to_load = (load_size / fragment_size);
  mvpd_uint16 remainder = (load_size % fragment_size);
  mvpd_uint32 i;
  
  if(position < (fragments_to_load + 2))
  {
    fseek(inputfile, audio_offset, SEEK_SET);
    if(10240 != (fread(compressed_audio, sizeof(mvpd_int8), 10240, inputfile)))
      return -1;
    MVPD_DECOMPRESS_AUDIO2(compressed_audio, 10240, audio_buffer);
    if((write(audio_dev, audio_buffer, (2 * 10240))) != (2 * 10240))
      return -1;
    if((10240 - fragment_size) != (fread(compressed_audio, sizeof(mvpd_int8), (10240 - fragment_size), inputfile)))
      return -1;
    MVPD_DECOMPRESS_AUDIO2(compressed_audio, (10240 - fragment_size), audio_buffer);
    if((write(audio_dev, audio_buffer, (2 * (10240 - fragment_size)))) != (2 * (10240 - fragment_size)))
      return -1;
    fseek(inputfile, start_offset, SEEK_SET);
    return 1;
  }
  
  
  fseek(inputfile, frame_index[(position - fragments_to_load - 2)], SEEK_SET);
  if((fragment_size - remainder) != (fread(compressed_audio, sizeof(mvpd_int8), (fragment_size - remainder), inputfile)))
    exit(-1);
  if(remainder != (fread(compressed_audio, sizeof(mvpd_int8), remainder, inputfile)))
    exit(-1);
  MVPD_DECOMPRESS_AUDIO2(compressed_audio, remainder, audio_buffer);
  if((write(audio_dev, audio_buffer, (2 * remainder))) != (2 * remainder))
    exit(-1);
  
  for(i = (position - fragments_to_load - 1); i < (position - 1); i++)
  {
    fseek(inputfile, frame_index[i], SEEK_SET);
    
    if((fragment_size + 1) != (fread(compressed_audio, sizeof(mvpd_int8), fragment_size+1, inputfile)))
      return -1;
    if(compressed_audio[fragment_size] != MVPD_END_OF_AUDIO_FRAME)
    {
      printf("\nError while reading audio data!");
#ifdef DEBUG
      fprintf(debug_log,"\nError while reading audio data...\n");
#endif
      exit(-1);
    }
    MVPD_DECOMPRESS_AUDIO2(compressed_audio, fragment_size, audio_buffer);
    if((write(audio_dev, audio_buffer, (2 * fragment_size))) != (2 * fragment_size))
      exit(-1);
  }
  fseek(inputfile, frame_index[position], SEEK_SET);
  
  return position;
}

