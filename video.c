#include <stdio.h>
#include <stdlib.h>

#include "mvpd.h"

mvpd_int16* block_idct_workspace = NULL;
mvpd_int16* zigzag_temp = NULL;
mvpd_int16* zigzag_dct = NULL;

void MVPD_INIT_DECODING()
{
  block_idct_workspace = (mvpd_int16*) calloc(64, sizeof(mvpd_int16*));	/* buffers data between passes */
  if(block_idct_workspace == NULL)
  {
    printf("\nError: Couldn't allocate memory for workspace buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for workspace buffer\n");
#endif
    exit(0);
  }

  zigzag_temp = (mvpd_int16*) calloc(64, sizeof(mvpd_int16));
  if(zigzag_temp == NULL)
  {
    printf("\nError: Couldn't allocate memory for zigzag buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for zigzag_temp buffer\n");
#endif
    exit(0);
  }

  zigzag_dct = (mvpd_int16*) calloc(64, sizeof(mvpd_int16));
  if(zigzag_dct == NULL)
  {
    printf("\nError: Couldn't allocate memory for dct buffer\n");
#ifdef DEBUG
    fprintf(debug_log,"\nError: Couldn't allocate memory for dct buffer\n");
#endif
    exit(0);
  }
}

void MVPD_CLOSE_DECODING()
{
  if(block_idct_workspace != NULL)
    free(block_idct_workspace);
  if(zigzag_temp != NULL)
    free(zigzag_temp);
  if(zigzag_dct != NULL)
    free(zigzag_dct);
}


void MVPD_RGB_ARRAY_TO_YUV_ARRAY(mvpd_uint8 RGB[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered RGB Array to YUV Array: ");
   fflush(debug_log);
#endif

  mvpd_int16 temp;
  mvpd_uint8 R,G,B;

  mvpd_uint32 max_x = 3 * matrix_x;
  mvpd_uint32 max_y = matrix_y * max_x;
  mvpd_uint32 i;

  for(i = 0; i < max_y; i += 3)
  {
    R = RGB[i+0];
    G = RGB[i+1];
    B = RGB[i+2];
    temp = (((66 * R) + (129 * G) + (25 * B) +128) / 256) + 16;
    RGB[i+0] = (temp);

    temp = (((-38 * R) - (74 * G) + (112 * B) +128) / 256) + 128;
    RGB[i+1] = (temp);

    temp = (((112 * R) - (94 * G) - (18 * B) +128) / 256) + 128;
    RGB[i+2] = (temp);
  }
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nRGB Array to YUV Array completed");
   fflush(debug_log);
#endif
}

void MVPD_YUV_ARRAY_TO_RGB_ARRAY(mvpd_uint8 YUV[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered YUV Array to RGB Array:");
   fflush(debug_log);
#endif

  mvpd_int16 temp;
  mvpd_uint16 C;
  mvpd_int8 D, E;

  mvpd_uint32 max_y = 3* matrix_y * matrix_x;
  mvpd_uint32 i;

  for(i = 0; i < max_y; i += 3)
  {

    C = (YUV[i+0] - 16) * 298;
    D = YUV[i+1] - 128;
    E = YUV[i+2] - 128;

    temp = ((C + (409 * E) + 128) >> 8);
    temp = abs(temp) & 248;
    YUV[i+0] = (temp);

    temp = ((C - (100 * D) - (208 * E) + 128) >> 8);
    temp = abs(temp) & 252;
    YUV[i+1] = (temp);

    temp = ((C + (516 * D) + 128) >> 8);
    temp = abs(temp) & 248;
    YUV[i+2] = (temp);
  }
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nYUV Array to RGB Array sucessfully completed.");
   fflush(debug_log);
#endif

}

void MVPD_YUV_ARRAY_TO_RGB565_ARRAY(mvpd_uint8 YUV[], mvpd_uint16 length, mvpd_uint16 RGB[])
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered YUV Array to RGB565 Array: ");
   fflush(debug_log);
#endif

#ifndef IPOD
  mvpd_int16 temp;
  mvpd_uint16 C;
  mvpd_int8 D, E;

  mvpd_uint32 i;

  for(i = 0; i < length; i++)
  {

    C = (YUV[(3 * i) + 0] - 16) * 298;
    D = YUV[(3 * i) + 1] - 128;
    E = YUV[(3 * i) + 2] - 128;

    temp = ((C + (409 * E) + 128) >> 8);
    temp = abs(temp) & 255;
    RGB[i] = (temp & 0xf8) << 8;

    temp = ((C - (100 * D) - (208 * E) + 128) >> 8);
    temp = abs(temp) & 255;
    RGB[i] += (temp & 0xfc) << 3;

    temp = ((C + (516 * D) + 128) >> 8);
    temp = abs(temp) & 255;
    RGB[i] += (temp & 0xf8) >> 3;
  }

#else
  mvpd_uint8 * end;
  end = YUV;
  end += (3 * length);

  asm volatile (
      ""
      "mov r1, #0xA\n\t"      /*r1  is 298 = 0x12A for multiplication */
      "orr r1, r1, #0x120\n\t"
      "mov r2, #0x19\n\t"     /*r2  is 409 = 0x199 for multiplication */
      "orr r2, r2, #0x180\n\t"
      "mov r3, #0x64\n\t"     /*r3  is 100 = 0x64 for multiplication */
      "mov r4, #0xD0\n\t"     /*r4 is 208 = 0xD0 for multiplication */
      "mov r5, #0x4\n\t"      /*r5 is 516 = 0x204 for multiplication */
      "orr r5, r5, #0x200\n\t"

      "1:\t"
      "ldrb r6, [%[YUV]], #1\n\t"  /*r6 = Y*/
      "ldrb r7, [%[YUV]], #1\n\t"  /*r7 = U*/
      "ldrb r8, [%[YUV]], #1\n\t"  /*r8 = V*/

      "sub r6, r6, #0x10\n\t" /*Substract  16 from Y to prepare C */
      "sub r7, r7, #0x80\n\t" /*Substract 128 from U to prepare D */
      "sub r8, r8, #0x80\n\t" /*Substract 128 from V to prepare E */

      "mul r6, r6, r1\n\t"    /*C is always multiplied by 298 */

      /* Calculate the Red Value in temporary variable r0 */
      /*  R = ((C + (409 * E) + 128) >> 8) */
      "mul r0, r8, r2\n\t"      /* E x 409 */
      "add r0, r0, r6\n\t"      /* Add C */
      "add r0, r0, #128\n\t"    /* Add 128 this is the final R Value, left shifted by 8 Bits */
      "cmp r0, #0\n\t"          /* Check if the result is negative */
      "movlt r0, #0\n\t"        /* If r0 is negative change r0 to zero */
      "cmp r0, #0xFF00\n\t"     /* Check if the result is bigger than 255 (still shifted by 8 Bit) */
      "movgt r0, #0xFF00\n\t"   /* If r0 is too high change r0 to 255 (still shifted by 8 Bit)*/
      "and r9, r0, #0xF800\n\t" /* Remove the 3 least significant bits and put into output variable r9*/

      /* Calculate the Green Value in temporary variable r0 */
      /* G = ((C - (100 * D) - (208 * E) + 128) >> 8) */
      "mul r8, r8, r4\n\t"         /* E x 208 stored in r8, since E isn't needed anymore*/
      "mul r0, r7, r3\n\t"         /* r0 = D x 100 */
      "sub r0, r6, r0\n\t"         /* r0 = C - (D * 100) */
      "sub r0, r0, r8\n\t"         /* r0 = C - (D * 100) - (208 * E) */
      "add r0, r0, #128\n\t"       /* r0 = C - (D * 100) - (208 * E) + 128 */
      "cmp r0, #0\n\t"             /* Check if the result is negative */
      "movlt r0, #0\n\t"           /* If r0 is negative change r0 to zero */
      "cmp r0, #0xFF00\n\t"        /* Check if the result is biger than 255 */
      "movgt r0, #0xFF00\n\t"      /* If r0 is too high change r0 to 255 */
      "and r0, r0, #0xFC00\n\t"    /* Remove the 2 least significant bits */

      "orr r9, r9, r0, lsr #5\n\t" /* Copy the Green Value in Bits 5-11 in r9 */

      /* Calculate the Blue Value in temporary variable r0 */
      /*   B = ((C + (516 * D) + 128) >> 8) */
      "mul r0, r7, r5\n\t"          /* r0 = D x 516 */
      "add r0, r0, r6\n\t"          /* r0 = C + (D x 516) */
      "add r0, r0, #128\n\t"        /* r0 = C + (D x 516) + 128 */
      "cmp r0, #0\n\t"              /* Check if the result is negative */
      "movlt r0, #0\n\t"            /* If r0 is negative change r0 to zero */
      "cmp r0, #0xFF00\n\t"         /* Check if the result is biger than 255 */
      "movgt r0, #0xFF00\n\t"       /* If r0 is too high change r0 to 255 */
      "and r0, r0, #0xF800\n\t"     /* Remove the 3 least significant bits */

      "orr r9, r9, r0, lsr #11\n\t" /* Copy the Green Value in Bits 5-11 in r9 */
      "strh r9, [%[RGB]], #2\n\t"   /* Store the 16 Bit RGB Value in the output matrix*/

      "cmp   %[YUV], %[end]\n\t"    /* Check to see wether we are at the end of the input */
      "bcc   1b"


  :: [YUV] "r" (YUV), [RGB] "r" (RGB), [end] "r" (end)
  : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9" );

#endif
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nYUV Array to RGB565 Array succesfully completed.");
  fflush(debug_log);
#endif
  return;
}



void CHROMA_SUBSAMPLING(mvpd_uint8 input[], mvpd_int16 Y[], mvpd_int16 U[], mvpd_int16 V[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered Chroma Subsampling: ");
  fflush(debug_log);
#endif

  mvpd_uint16 temp;
  mvpd_uint32 max_x = 3 * matrix_x;
  mvpd_uint32 max_y = matrix_y * max_x;

  bool even_x = true;
  bool even_y = false;

  mvpd_uint32 yuv_counter = 0;
  mvpd_uint32 y_counter = 0;
  mvpd_uint32 u_counter = 0;
  mvpd_uint32 v_counter = 0;

  for(yuv_counter = 0; yuv_counter < max_y; yuv_counter += 3)
  {
    Y[y_counter++] = input[yuv_counter+0];

    if((yuv_counter % max_x) == 0)
      even_y = 1 - even_y;

    if (even_x)
    {
      if (even_y)
      {
        temp = (input[yuv_counter + 1] + input[yuv_counter + max_x + 1] + input[yuv_counter + 4] + input[yuv_counter + max_x + 4]) / 4;
        U[u_counter++] = temp;
      }
      else
      {
        temp = (input[yuv_counter - max_x + 2] + input[yuv_counter - max_x + 5] + input[yuv_counter + 2] + input[yuv_counter + 5]) / 4;
        V[v_counter++] = temp;
      }
      even_x = 0;
    }
    else even_x = 1;
  }
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nChroma Subsampling completed sucessfully.");
  fflush(debug_log);
#endif
}

void REVERSE_CHROMA_SUBSAMPLING(mvpd_uint8 Y[], mvpd_uint8 U[], mvpd_uint8 V[], mvpd_uint8 output[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered Reverse Chroma Subsampling: ");
  fflush(debug_log);
#endif

  mvpd_uint32 yuv_counter = 0;
  mvpd_uint32 y_counter = 0;
  mvpd_uint32 uv_counter = 0;

  mvpd_uint8 C, D, E;

  mvpd_uint16 row_length = matrix_x*3;
  mvpd_uint16 row = 0;


  while(yuv_counter < 3*(matrix_x * matrix_y))
  {
    for(row = 0; (row < matrix_x / 2); row++)
    {
      C = Y[y_counter++];
      if(C < 16)
        C = 16;
      else
        if(C > 235)
          C = 235;

      D = U[uv_counter];
      if(D < 16)
        D = 16;
      else
        if(D > 240)
          D = 240;
      E = V[uv_counter++];
      if(E < 16)
        E = 16;
      else
        if(E > 240)
          E = 240;

      output[yuv_counter++] = C;
      output[yuv_counter + row_length] = D;
      output[yuv_counter++] = D;
      output[yuv_counter + row_length] = E;
      output[yuv_counter++] = E;

      C = Y[y_counter++];
      if(C < 16)
        C = 16;
      else
        if(C > 235)
          C = 235;

      output[yuv_counter++] = C;
      output[yuv_counter + row_length] = D;
      output[yuv_counter++] = D;
      output[yuv_counter + row_length] = E;
      output[yuv_counter++] = E;


    }

    for(row = 0; (row < matrix_x); row++)
    {
      C = Y[y_counter++];
      if(C < 16)
        C = 16;
      else
        if(C > 235)
          C = 235;

      output[yuv_counter] = C;
      yuv_counter += 3;


    }
  }

#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nReverse Chroma Subsampling completed sucessfully.");
  fflush(debug_log);
#endif
}


/*
The Functions MVPD_DCT and MVPD_IDCT are mostly copied from the
Independent JPEG Groups implementation. (www.ijg.org)
There are some minor changes since i tried to understand the concept
and rewrote each function a few times.
Basically those functions are an Implementation Arai, Agui and
Nakajima's algorithm for the fast DCT as outlined in the "JPEG
Textbook" by Pennebaker and Mitchell.
The original code can be obtained from http://www.ijg.org/files
However i hope that we can get a assembler implementation of the
IDCT for faster playback on the IPOD */

#define FIX_0_382683433  ((long)   98)		/* FIX(0.382683433) */
#define FIX_0_541196100  ((long)  139)		/* FIX(0.541196100) */
#define FIX_0_707106781  ((long)  181)		/* FIX(0.707106781) */
#define FIX_1_306562965  ((long)  334)		/* FIX(1.306562965) */

#define FIX_1_082392200  ((long)  277)		/* FIX(1.082392200) */
#define FIX_1_414213562  ((long)  362)		/* FIX(1.414213562) */
#define FIX_1_847759065  ((long)  473)		/* FIX(1.847759065) */
#define FIX_2_613125930  ((long)  669)		/* FIX(2.613125930) */

#define fixed_mul(x, y) ((x * y) >> 8)


void MVPD_SHIFT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
  int i;
  for(i = 0; i < (matrix_y * matrix_x); i++)
    input[i] = input[i] - 128;
}

void MVPD_ISHIFT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
  int i;
  for(i = 0; i < (matrix_y * matrix_x); i++)
    input[i] = input[i] + 128;
}

void MVPD_DCT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
  mvpd_int16 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  mvpd_int16 tmp10, tmp11, tmp12, tmp13;
  mvpd_int16 z1, z2, z3, z4, z5, z11, z13;

  mvpd_int16 i,j;
  mvpd_int16 row;

  /* Pass 1: process rows. */
  for(j = 0; j < matrix_x; j += 8)
    for (i = 0; i < matrix_y; i++)
  {
    row = i * matrix_x;
    tmp0 = input[row+j+0] + input[row+j+7];
    tmp7 = input[row+j+0] - input[row+j+7];
    tmp1 = input[row+j+1] + input[row+j+6];
    tmp6 = input[row+j+1] - input[row+j+6];
    tmp2 = input[row+j+2] + input[row+j+5];
    tmp5 = input[row+j+2] - input[row+j+5];
    tmp3 = input[row+j+3] + input[row+j+4];
    tmp4 = input[row+j+3] - input[row+j+4];

    /* Even part */
    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    input[row+j+0] = tmp10 + tmp11; /* phase 3 */
    input[row+j+4] = tmp10 - tmp11;

    z1 = fixed_mul((tmp12 + tmp13), FIX_0_707106781); /* c4 */

    input[row+j+2] = tmp13 + z1;	/* phase 5 */
    input[row+j+6] = tmp13 - z1;

    /* Odd part */

    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = fixed_mul((tmp10 - tmp12), FIX_0_382683433); /* c6 */
    z2 = fixed_mul(tmp10 , FIX_0_541196100) + z5; /* c2-c6 */
    z4 = fixed_mul(tmp12 , FIX_1_306562965) + z5; /* c2+c6 */
    z3 = fixed_mul(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;

    input[row+j+5] = z13 + z2;	/* phase 6 */
    input[row+j+3] = z13 - z2;
    input[row+j+1] = z11 + z4;
    input[row+j+7] = z11 - z4;
  }

  /* Pass 2: process columns. */
  for(j = 0; j < matrix_y; j += 8)
    for (i = 0; i < matrix_x; i++)
  {
    tmp0 = input[(j+0)*matrix_x + i] + input[(j+7)*matrix_x + i];
    tmp7 = input[(j+0)*matrix_x + i] - input[(j+7)*matrix_x + i];
    tmp1 = input[(j+1)*matrix_x + i] + input[(j+6)*matrix_x + i];
    tmp6 = input[(j+1)*matrix_x + i] - input[(j+6)*matrix_x + i];
    tmp2 = input[(j+2)*matrix_x + i] + input[(j+5)*matrix_x + i];
    tmp5 = input[(j+2)*matrix_x + i] - input[(j+5)*matrix_x + i];
    tmp3 = input[(j+3)*matrix_x + i] + input[(j+4)*matrix_x + i];
    tmp4 = input[(j+3)*matrix_x + i] - input[(j+4)*matrix_x + i];

    /* Even part */
    tmp10 = tmp0 + tmp3;	/* phase 2 */
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    input[(j+0)*matrix_x + i] = (tmp10 + tmp11) >> 3; /* phase 3 */
    input[(j+4)*matrix_x + i] = (tmp10 - tmp11) >> 3;

    z1 = fixed_mul((tmp12 + tmp13) ,FIX_0_707106781); /* c4 */
    input[(j+2)*matrix_x + i] = (tmp13 + z1) >> 3; /* phase 5 */
    input[(j+6)*matrix_x + i] = (tmp13 - z1) >> 3;

    /* Odd part */
    tmp10 = tmp4 + tmp5;	/* phase 2 */
    tmp11 = tmp5 + tmp6;
    tmp12 = tmp6 + tmp7;

    /* The rotator is modified from fig 4-8 to avoid extra negations. */
    z5 = fixed_mul((tmp10 - tmp12), FIX_0_382683433); /* c6 */
    z2 = fixed_mul(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
    z4 = fixed_mul(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
    z3 = fixed_mul(tmp11, FIX_0_707106781); /* c4 */

    z11 = tmp7 + z3;		/* phase 5 */
    z13 = tmp7 - z3;

    input[(j+5)*matrix_x + i] = (z13 + z2) >> 3; /* phase 6 */
    input[(j+3)*matrix_x + i] = (z13 - z2) >> 3;
    input[(j+1)*matrix_x + i] = (z11 + z4) >> 3;
    input[(j+7)*matrix_x + i] = (z11 - z4) >> 3;
  }

}

void MVPD_IDCT(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nIDCT betreten X: %i  Y: %i.", matrix_x, matrix_y);
  fflush(debug_log);
#endif

  mvpd_int16 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  mvpd_int16 tmp10, tmp11, tmp12, tmp13;
  mvpd_int16 z5, z10, z11, z12, z13;
  mvpd_int16 row;

  mvpd_int16* workspace;	/* buffers data between passes */
  workspace = (mvpd_int16*) calloc(matrix_x*matrix_y, sizeof(mvpd_int16));

  mvpd_int16 i,j;


  for(j = 0; j < matrix_y; j += 8)
    for (i = 0; i < matrix_x; i++)
  {
    /* Due to quantization, we will usually find that many of the input
    * coefficients are zero, especially the AC terms.  We can exploit this
    * by short-circuiting the IDCT calculation for any column in which all
    * the AC terms are zero.  In that case each output is equal to the
    * DC coefficient (with scale factor as needed).
    * With typical images and quantization tables, half or more of the
    * column DCT calculations can be simplified this way.
    */
    if (input[(j+1)*matrix_x + i] == 0 && input[(j+2)*matrix_x + i] == 0 &&
        input[(j+3)*matrix_x + i] == 0 && input[(j+4)*matrix_x + i] == 0 &&
        input[(j+5)*matrix_x + i] == 0 && input[(j+6)*matrix_x + i] == 0 &&
        input[(j+7)*matrix_x + i] == 0)
    {
      tmp0 = input[(j+0)*matrix_x + i];
      workspace[(j+0)*matrix_x + i] = tmp0;
      workspace[(j+1)*matrix_x + i] = tmp0;
      workspace[(j+2)*matrix_x + i] = tmp0;
      workspace[(j+3)*matrix_x + i] = tmp0;
      workspace[(j+4)*matrix_x + i] = tmp0;
      workspace[(j+5)*matrix_x + i] = tmp0;
      workspace[(j+6)*matrix_x + i] = tmp0;
      workspace[(j+7)*matrix_x + i] = tmp0;

      continue;
    }

    /* Even part */
    tmp0 = input[(j+0)*matrix_x + i];
    tmp1 = input[(j+2)*matrix_x + i];
    tmp2 = input[(j+4)*matrix_x + i];
    tmp3 = input[(j+6)*matrix_x + i];

    tmp10 = tmp0 + tmp2;	/* phase 3 */
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	/* phases 5-3 */
    tmp12 = fixed_mul((tmp1 - tmp3), FIX_1_414213562) - tmp13; /* 2*c4 */

    tmp0 = tmp10 + tmp13;	/* phase 2 */
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    tmp4 = input[(j+1)*matrix_x + i];
    tmp5 = input[(j+3)*matrix_x + i];
    tmp6 = input[(j+5)*matrix_x + i];
    tmp7 = input[(j+7)*matrix_x + i];

    z13 = tmp6 + tmp5;		/* phase 6 */
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = fixed_mul((z11 - z13), FIX_1_414213562); /* 2*c4 */

    z5 = fixed_mul((z10 + z12), FIX_1_847759065); /* 2*c2 */
    tmp10 = fixed_mul(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = fixed_mul(z10, (- FIX_2_613125930)) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    workspace[(j+0)*matrix_x + i] = (mvpd_int16) (tmp0 + tmp7);
    workspace[(j+7)*matrix_x + i] = (mvpd_int16) (tmp0 - tmp7);
    workspace[(j+1)*matrix_x + i] = (mvpd_int16) (tmp1 + tmp6);
    workspace[(j+6)*matrix_x + i] = (mvpd_int16) (tmp1 - tmp6);
    workspace[(j+2)*matrix_x + i] = (mvpd_int16) (tmp2 + tmp5);
    workspace[(j+5)*matrix_x + i] = (mvpd_int16) (tmp2 - tmp5);
    workspace[(j+4)*matrix_x + i] = (mvpd_int16) (tmp3 + tmp4);
    workspace[(j+3)*matrix_x + i] = (mvpd_int16) (tmp3 - tmp4);
  }

  if(MVPD_DEBUG)
  {
    printf("\nRows fertig, starte Columns.");
    fflush(stdout);
  }


  for(j = 0; j < matrix_x; j += 8)
    for (i = 0; i < matrix_y; i++)
  {
    row = i * matrix_x;
    /* Rows of zeroes can be exploited in the same way as we did with columns.
    * However, the column calculation has created many nonzero AC terms, so
    * the simplification applies less often (typically 5% to 10% of the time).
    * On machines with very fast multiplication, it's possible that the
    * test takes more time than it's worth.  In that case this section
    * may be commented out.
    */

/*    if (workspace[row + j+1] == 0 && workspace[row + j+2] == 0 && workspace[row + j+3] == 0 && workspace[row + j+4] == 0  && workspace[row + j+5] == 0 && workspace[row + j+6] == 0 && workspace[row + j+7] == 0)
    {
    tmp0 = input[row + j+0] = (workspace[row + j+0]) >> 3;
    input[row + j+0] = tmp0;
    input[row + j+1] = tmp0;
    input[row + j+2] = tmp0;
    input[row + j+3] = tmp0;
    input[row + j+4] = tmp0;
    input[row + j+5] = tmp0;
    input[row + j+6] = tmp0;
    input[row + j+7] = tmp0;
    continue;
  }
  */
    /* Even part */

    tmp10 = workspace[row + j+0] + workspace[row + j+4];
    tmp11 = workspace[row + j+0] - workspace[row + j+4];

    tmp13 = workspace[row + j+2] + workspace[row + j+6];
    tmp12 = fixed_mul((workspace[row + j+2] - workspace[row + j+6]), FIX_1_414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = workspace[row + j+5] + workspace[row + j+3];
    z10 = workspace[row + j+5] - workspace[row + j+3];
    z11 = workspace[row + j+1] + workspace[row + j+7];
    z12 = workspace[row + j+1] - workspace[row + j+7];

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = fixed_mul((z11 - z13), FIX_1_414213562); /* 2*c4 */

    z5 = fixed_mul((z10 + z12), FIX_1_847759065); /* 2*c2 */
    tmp10 = fixed_mul(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = fixed_mul(z10, (- FIX_2_613125930)) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    input[row + j+0] = (tmp0 + tmp7) >> 3;
    input[row + j+7] = (tmp0 - tmp7) >> 3;
    input[row + j+1] = (tmp1 + tmp6) >> 3;
    input[row + j+6] = (tmp1 - tmp6) >> 3;
    input[row + j+2] = (tmp2 + tmp5) >> 3;
    input[row + j+5] = (tmp2 - tmp5) >> 3;
    input[row + j+4] = (tmp3 + tmp4) >> 3;
    input[row + j+3] = (tmp3 - tmp4) >> 3;
  }

  free(workspace);

  if(MVPD_DEBUG)
  {
    printf("\nVerlasse IDCT.");
    fflush(stdout);
  }
  free(workspace);
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nSucessfully Completed.");
  fflush(debug_log);
#endif
  return;
}

void MVPD_BLOCK_IDCT(mvpd_int16 input[])
{

  mvpd_int16 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
  mvpd_int16 tmp10, tmp11, tmp12, tmp13;
  mvpd_int16 z5, z10, z11, z12, z13;

  mvpd_int16 i;


  for(i = 0; i <8; i++)
  {
    /* Due to quantization, we will usually find that many of the input
    * coefficients are zero, especially the AC terms.  We can exploit this
    * by short-circuiting the IDCT calculation for any column in which all
    * the AC terms are zero.  In that case each output is equal to the
    * DC coefficient (with scale factor as needed).
    * With typical images and quantization tables, half or more of the
    * column DCT calculations can be simplified this way.
    */
    if (input[i+8] == 0 && input[i+16] == 0 &&
        input[i+24] == 0 && input[i+32] == 0 &&
        input[i+40] == 0 && input[i+48] == 0 &&
        input[i+56] == 0)
    {
      tmp0 = input[i];
      block_idct_workspace[i] = tmp0;
      block_idct_workspace[i+8] = tmp0;
      block_idct_workspace[i+16] = tmp0;
      block_idct_workspace[i+24] = tmp0;
      block_idct_workspace[i+32] = tmp0;
      block_idct_workspace[i+40] = tmp0;
      block_idct_workspace[i+48] = tmp0;
      block_idct_workspace[i+56] = tmp0;

      continue;
    }

    /* Even part */
    tmp0 = input[i];
    tmp1 = input[i+16];
    tmp2 = input[i+32];
    tmp3 = input[i+48];

    tmp10 = tmp0 + tmp2;	/* phase 3 */
    tmp11 = tmp0 - tmp2;

    tmp13 = tmp1 + tmp3;	/* phases 5-3 */
    tmp12 = (((tmp1 - tmp3) * FIX_1_414213562) >> 8) - tmp13; /* 2*c4 */

    tmp0 = tmp10 + tmp13;	/* phase 2 */
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    tmp4 = input[i+8];
    tmp5 = input[i+24];
    tmp6 = input[i+40];
    tmp7 = input[i+56];

    z13 = tmp6 + tmp5;		/* phase 6 */
    z10 = tmp6 - tmp5;
    z11 = tmp4 + tmp7;
    z12 = tmp4 - tmp7;

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = fixed_mul((z11 - z13), FIX_1_414213562); /* 2*c4 */

    z5 = fixed_mul((z10 + z12), FIX_1_847759065); /* 2*c2 */
    tmp10 = fixed_mul(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = fixed_mul(z10, (- FIX_2_613125930)) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    block_idct_workspace[i] = (mvpd_int16) (tmp0 + tmp7);
    block_idct_workspace[i+56] = (mvpd_int16) (tmp0 - tmp7);
    block_idct_workspace[i+8] = (mvpd_int16) (tmp1 + tmp6);
    block_idct_workspace[i+48] = (mvpd_int16) (tmp1 - tmp6);
    block_idct_workspace[i+16] = (mvpd_int16) (tmp2 + tmp5);
    block_idct_workspace[i+40] = (mvpd_int16) (tmp2 - tmp5);
    block_idct_workspace[i+32] = (mvpd_int16) (tmp3 + tmp4);
    block_idct_workspace[i+24] = (mvpd_int16) (tmp3 - tmp4);
  }


  for(i = 0; i < 64; i += 8)
  {
    /* Rows of zeroes can be exploited in the same way as we did with columns.
    * However, the column calculation has created many nonzero AC terms, so
    * the simplification applies less often (typically 5% to 10% of the time).
    * On machines with very fast multiplication, it's possible that the
    * test takes more time than it's worth.  In that case this section
    * may be commented out.
    */

    if (block_idct_workspace[i+1] == 0 && block_idct_workspace[i+2] == 0 &&
        block_idct_workspace[i+3] == 0 && block_idct_workspace[i+4] == 0 &&
        block_idct_workspace[i+5] == 0 && block_idct_workspace[i+6] == 0 &&
        block_idct_workspace[i+7] == 0)
    {
      tmp0 = (block_idct_workspace[i] >> 3) + 128;
      input[i] = tmp0;
      input[i+1] = tmp0;
      input[i+2] = tmp0;
      input[i+3] = tmp0;
      input[i+4] = tmp0;
      input[i+5] = tmp0;
      input[i+6] = tmp0;
      input[i+7] = tmp0;

      continue;
    }

    /* Even part */

    tmp10 = block_idct_workspace[i+0] + block_idct_workspace[i+4];
    tmp11 = block_idct_workspace[i+0] - block_idct_workspace[i+4];

    tmp13 = block_idct_workspace[i+2] + block_idct_workspace[i+6];
    tmp12 = fixed_mul((block_idct_workspace[i+2] - block_idct_workspace[i+6]), FIX_1_414213562) - tmp13;

    tmp0 = tmp10 + tmp13;
    tmp3 = tmp10 - tmp13;
    tmp1 = tmp11 + tmp12;
    tmp2 = tmp11 - tmp12;

    /* Odd part */

    z13 = block_idct_workspace[i+5] + block_idct_workspace[i+3];
    z10 = block_idct_workspace[i+5] - block_idct_workspace[i+3];
    z11 = block_idct_workspace[i+1] + block_idct_workspace[i+7];
    z12 = block_idct_workspace[i+1] - block_idct_workspace[i+7];

    tmp7 = z11 + z13;		/* phase 5 */
    tmp11 = fixed_mul((z11 - z13), FIX_1_414213562); /* 2*c4 */

    z5 = fixed_mul((z10 + z12), FIX_1_847759065); /* 2*c2 */
    tmp10 = fixed_mul(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
    tmp12 = fixed_mul(z10, (- FIX_2_613125930)) + z5; /* -2*(c2+c6) */

    tmp6 = tmp12 - tmp7;	/* phase 2 */
    tmp5 = tmp11 - tmp6;
    tmp4 = tmp10 + tmp5;

    /* Final output stage: scale down by a factor of 8 and range-limit */

    input[i+0] = ((tmp0 + tmp7) >> 3) + 128;
    input[i+7] = ((tmp0 - tmp7) >> 3) + 128;
    input[i+1] = ((tmp1 + tmp6) >> 3) + 128;
    input[i+6] = ((tmp1 - tmp6) >> 3) + 128;
    input[i+2] = ((tmp2 + tmp5) >> 3) + 128;
    input[i+5] = ((tmp2 - tmp5) >> 3) + 128;
    input[i+4] = ((tmp3 + tmp4) >> 3) + 128;
    input[i+3] = ((tmp3 - tmp4) >> 3) + 128;
  }
  return;
}


//-------------Quant Zeug
//These are the values as recommended by the JPEG Standard
static const mvpd_uint8  const_quant_liste[2][64]={{16,11,12,14,12,10,16,14,13,14,18,17,16,19,24,40,26,24,22,22,24,49,35,37,29,40,58,51,61,60,57,51,56,55,64,72,92,78,64,68,87,69,55,56,80,109,81,87,95,98,103,104,103,62,77,113,121,112,100,120,92,101,103,99},{17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99}};
static mvpd_uint8  quant_liste[2][64];

//Helper Functions
mvpd_uint16 MVPD_MAX(mvpd_uint16 first, mvpd_uint16 second)
{
  if(first > second)
    return first;
  return second;
}

mvpd_uint16 MVPD_MIN(mvpd_uint16 first, mvpd_uint16 second)
{
  if(first < second)
    return first;
  return second;
}

void MVPD_INITIALIZE_QUANT(mvpd_uint8 quality)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nInitializing the quantaziation table: ");
  fflush(debug_log);
#endif
  mvpd_uint8 i;
  for(i = 0; i < 64; i++)
  {
    quant_liste[0][i] = MVPD_MIN(255, MVPD_MAX(9, (const_quant_liste[0][i] * quality) / 15));
    quant_liste[1][i] = MVPD_MIN(255, MVPD_MAX(9, (const_quant_liste[1][i] * quality) / 15));
    if(MVPD_DEBUG)
    {
      printf("\nQuant Liste % i auf %i und %i gesetzt.", i, quant_liste[0][i], quant_liste[1][i]);
      fflush(stdout);
    }
  }
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"Quantization table initiliased");
  fflush(debug_log);
#endif
  return;
}

//This function isn't used anymore, it has been replaced by REVERSE_ZIGZAG_BLOCK
void MVPD_REVERSE_ZZ(mvpd_int16* input, mvpd_int16* output)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered Reverse ZZ: ");
  fflush(debug_log);
#endif

  int index = 0;
  int y = 0;
  int x = 0;
  int v = 1;

  while((x != 7) || (y != 7))
  {
    output[(y) * 8 + x] = input[index];
    index++;

    if(((y == 0) && ( v > 0)) || ((y == 7) && ( v < 0)))
    {
      x++;
      v = -v;
      continue;
    }

    if(((x == 0) && ( v < 0)) || ((x == 7) && ( v > 0)))
    {
      y++;
      v = -v;
      continue;
    }

    x += v;
    y -= v;
  }
  output[(y) * 8 + x] = input[index];
  return;

#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"Reverse ZZ completed.");
  fflush(debug_log);
#endif
}

//Reverses the zigzag encoding fo a 8x8 Block
void MVPD_REVERSE_ZIGZAG_BLOCK(mvpd_int16* input, mvpd_int16* outpout)
{
  outpout[0] = input[0];
  outpout[1] = input[1];
  outpout[2] = input[5];
  outpout[3] = input[6];
  outpout[4] = input[14];
  outpout[5] = input[15];
  outpout[6] = input[27];
  outpout[7] = input[28];
  outpout[8] = input[2];
  outpout[9] = input[4];
  outpout[10] = input[7];
  outpout[11] = input[13];
  outpout[12] = input[16];
  outpout[13] = input[26];
  outpout[14] = input[29];
  outpout[15] = input[42];
  outpout[16] = input[3];
  outpout[17] = input[8];
  outpout[18] = input[12];
  outpout[19] = input[17];
  outpout[20] = input[25];
  outpout[21] = input[30];
  outpout[22] = input[41];
  outpout[23] = input[43];
  outpout[24] = input[9];
  outpout[25] = input[11];
  outpout[26] = input[18];
  outpout[27] = input[24];
  outpout[28] = input[31];
  outpout[29] = input[40];
  outpout[30] = input[44];
  outpout[31] = input[53];
  outpout[32] = input[10];
  outpout[33] = input[19];
  outpout[34] = input[23];
  outpout[35] = input[32];
  outpout[36] = input[39];
  outpout[37] = input[47];
  outpout[38] = input[52];
  outpout[39] = input[54];
  outpout[40] = input[20];
  outpout[41] = input[22];
  outpout[42] = input[33];
  outpout[43] = input[38];
  outpout[44] = input[46];
  outpout[45] = input[51];
  outpout[46] = input[55];
  outpout[47] = input[60];
  outpout[48] = input[21];
  outpout[49] = input[34];
  outpout[50] = input[37];
  outpout[51] = input[47];
  outpout[52] = input[50];
  outpout[53] = input[56];
  outpout[54] = input[59];
  outpout[55] = input[61];
  outpout[56] = input[35];
  outpout[57] = input[36];
  outpout[58] = input[48];
  outpout[59] = input[49];
  outpout[60] = input[57];
  outpout[61] = input[58];
  outpout[62] = input[62];
  outpout[63] = input[63];
  return;
}




mvpd_uint16 MVPD_ZIGZAG(mvpd_int16 input[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y, mvpd_uint8 chrom,  mvpd_int8 output[], mvpd_uint32 position)
{

  if(MVPD_DEBUG)
    printf("\nEntered ZigZag:");

  mvpd_int16 temp[64];
  mvpd_uint8 index;
  mvpd_uint32 y,x, i, j; //countervariables
  mvpd_int16 a;
  mvpd_int16 v;
  mvpd_int8 ende;

  for(i = 0; i < matrix_y; i += 8)
    for(j = 0; j < matrix_x; j += 8)
  {
    index = 0;
    y = 0;
    x = 0;
    v = 1;
    while((x != 7) || (y != 7))
    {
      temp[index] = input[(i+y) * matrix_x + (j+x)] / quant_liste[chrom][index];
      if(temp[index] > 124)
      {
        printf("\nError: Sample Quality too high: %i will be cut too 124", temp[index]);
        temp[index] = 124;
      }
      if(temp[index] < -124)
      {
        printf("\nError: Sample Quality too high: %i will be cut too -124", temp[index]);
        temp[index] = -124;
      }
      index++;

      if(((y == 0) && ( v > 0)) || ((y == 7) && ( v < 0)))
      {
        x++;
        v = -v;
        continue;
      }

      if(((x == 0) && ( v < 0)) || ((x == 7) && ( v > 0)))
      {
        y++;
        v = -v;
        continue;
      }

      x += v;
      y -= v;
    }
    temp[index] = input[(i+y) * matrix_x + (j+x)] / quant_liste[chrom][index];

    ende = 63;
    while((temp[ende] == 0) && (ende >= 0))
      ende--;

    if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 2))
      printf("length %i\n", ende);

    for(a = 0; a <= ende; a++)
    {
      output[position++] = temp[a];
    }


    output[position++] = (MVPD_END_OF_PICTURE_BLOCK); //EOB Signal

  }

  output[position++] = MVPD_END_OF_PICTURE_COLOUR; //EOP Signal

  return position;
}



mvpd_uint16 MVPD_IZIGZAG(mvpd_int8 input[], mvpd_uint32 position, mvpd_uint8 chrom, mvpd_uint8 output[], mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nEntered IZigZag: ");
  fflush(debug_log);
#endif

  mvpd_uint8 index;
  mvpd_uint32 y,x, i, j; //countervariables
  mvpd_int16 a;


  for(i = 0; i < matrix_y; i += 8)
    for(j = 0; j < matrix_x; j += 8)
  {
    index = 0;

    while(input[position] != MVPD_END_OF_PICTURE_BLOCK)
    {
      if((MVPD_DEBUG) && (index > 63))
      {
        printf("\nError BLock with more than 63 values: %i at position %i", index, position);
        fflush(stdout);
      }
      zigzag_temp[index] = input[position] * quant_liste[chrom][index];

      position++;
      index++;
    }
    if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 2))
      printf("\nHier sollte eine 125 stehen: %i", input[position]);
    position++;

    for(a = index; a < 64; a++)
      zigzag_temp[a] = 0;

    MVPD_REVERSE_ZIGZAG_BLOCK(zigzag_temp, zigzag_dct);

#ifdef TEST
    _MVPD_ARM_COLUMN_DCT(zigzag_dct, block_idct_workspace);
    _MVPD_ARM_ROW_DCT(block_idct_workspace, zigzag_dct);
#else
    MVPD_BLOCK_IDCT(zigzag_dct);
#endif

    for(y = 0; y < 8; y++)
      for(x = 0; x < 8; x++)
        output[((i+y) * matrix_x) + (j+x)] = (mvpd_uint8) zigzag_dct[(y*8) + x];
  }


  if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 1))
    printf("\nHier sollte eine 126 stehen: %i", input[position]);
  position++;

#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"IZigZag completed.");
  fflush(debug_log);
#endif
  return position;
}


//Hilfsfunktionen: Mssen definitiv noch angepasst werden!!!
mvpd_uint32 hex2dword(unsigned char* c)
{
  mvpd_uint32 ergebnis;
  mvpd_int32 i;
  ergebnis = 0;
  for(i = 3; 0 <= i; i--)
    ergebnis = ergebnis * 256 + (mvpd_uint16) (c[i]);
  return ergebnis;
}

mvpd_uint16 hex2short(unsigned char* c)
{
  mvpd_uint16 ergebnis;
  ergebnis = 256 * (mvpd_uint16) (c[1]) + (mvpd_uint16) (c[0]);
  return ergebnis;
}

//Ist noch nicht fr negative Werte angepasst.
mvpd_int32 hex2long(unsigned char* c) //
{
  mvpd_int32 ergebnis = 0;
  mvpd_int32 i;
  for(i = 3; 0 <= i; i--)
    ergebnis = ergebnis * 256 + c[i];
  return ergebnis;
}



void INIT_BITMAP(BMP* bitmap)
{
  bitmap->picture = (mvpd_uint8 *) calloc((3 * bitmap->height * bitmap->width), sizeof(mvpd_uint8));
}



void LOAD_BITMAP(char* filename, BMP* bitmap, mvpd_uint16 matrix_x, mvpd_uint16 matrix_y)
{
  if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 0) )
    printf("\nEntering bmp Constructor");

  mvpd_int32 i, j, a;      //Schleifenvariablen
  char temp;
  unsigned char* int_buffer;   //Puffer um 32 Bit Zahlen einzulesen
  char* char_buffer;

  char_buffer = (char*) calloc(2, sizeof(char));
  int_buffer = (unsigned char*) calloc(4, sizeof(char));

  mvpd_uint16 colordepth, skew, pixeldword;
  mvpd_uint32 size;
  mvpd_uint16 offset;


  FILE *bitmapfile = fopen(filename, "rb");

  if (bitmapfile == NULL)
  {
    printf("Sorry the file %s could not be openend.\nThe program will be terminated", filename);
    return; //Here should be some error handling stuff
  }


  // Kontrollieren, ob es sich um eine passende Bitmapdatei handelt

  char_buffer = fgets(char_buffer, 3, bitmapfile);

  if (char_buffer[0] != 'B' || char_buffer[1] != 'M')
  {
    printf("\nSorry, can't open the file %s.\nThe program will be terminated.\n", filename);
    return;
  }

  if(fseek(bitmapfile, 8, SEEK_CUR) != 0)
    return;


  //Das Einlesen direkt als int klappt nicht, also lese ich den Wert erst als String ein, und wandele in dann um
  fread(int_buffer, sizeof(unsigned char), 4, bitmapfile);

  offset = hex2dword(int_buffer);



  //biSize ignorieren wir auch.
  if(fseek(bitmapfile, 4, SEEK_CUR) != 0)
    return;

  fread(int_buffer, sizeof(unsigned char), 4, bitmapfile);
  //Breite
  bitmap->width = hex2long(int_buffer);


  fread(int_buffer, sizeof(unsigned char), 4, bitmapfile);
  //Hoehe, zur Zeit unterstuetzt das Programm nur die Bottomup Variante mit positiven hoehen.
  bitmap->height = hex2long(int_buffer);

  //biPlanes ignorieren...
  if(fseek(bitmapfile, 2, SEEK_CUR) != 0)
    return;


  fread(int_buffer, sizeof(unsigned char), 2, bitmapfile);

  colordepth = hex2short(int_buffer);
  pixeldword = (32 / colordepth); //Gibt die Anzahl der Pixel pro dword an, bei 4 Bit = 8, bei 8 Bit =4
  skew = (pixeldword - (bitmap->width % pixeldword)) % pixeldword; //Das letzte Dword einer Zeile wird mit Nullen aufgefuellt, der Skew gibt an mit wievielen Nullen.


  if (colordepth != 24)
  {
    printf("\nSorry, must be a 24 Bit Bitmapfile.\nThe program will be terminated.\n");
    return;
  }

  fread(int_buffer, sizeof(unsigned char), 4, bitmapfile);
  if (hex2dword(int_buffer) != 0)
  {
    printf("\nSorry, compressed files aren't supported.\nThe program will be terminated.\n");
    return;
  }

  fread(int_buffer, sizeof(unsigned char), 4, bitmapfile);
  size=hex2dword(int_buffer);


  if (size != 0 && size != (unsigned int) ((bitmap->height * (bitmap->width + skew) * colordepth))/8) //Ich hab keine Ahnung, wieso das hier nur durch 6 geteilt werden muss, aber es funktioniert.
  {
    printf("\nSorry, the file size is incorrect.\nThe program will be terminated.\n");
    return;
  }

  //Der Rest des headers mvpd_interessiert uns nicht, also springen wir an den Anfang der Bilddaten:
  fseek(bitmapfile, offset, SEEK_SET);

  mvpd_uint16 input_y_skew = 0;
  if(matrix_y < bitmap->height)
  {
    input_y_skew = (((bitmap->height - matrix_y)/2) * bitmap->width * (colordepth / 8));
    bitmap->height = matrix_y;
    if(fseek(bitmapfile, input_y_skew, SEEK_CUR) != 0)
      return;
  }

  mvpd_uint16 output_y_skew = 0;
  if(matrix_y > bitmap->height)
  {
    output_y_skew = (matrix_y - bitmap->height) / 2;
  }

  if(matrix_x < bitmap->width)
  {
    if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 1))
    {
      printf("\nChanged skew, old skew: %i, width difference: %i, new skew: %i\n", skew, (bitmap->width - matrix_x), skew + (bitmap->width - matrix_x));
    }
    skew = skew + ((bitmap->width - matrix_x) * (colordepth / 8));
    bitmap->width = matrix_x;
  }

  mvpd_uint16 output_x_skew = 0;
  if(matrix_x > bitmap->width)
  {
    output_x_skew = matrix_x - bitmap->width;
  }

  if(fseek(bitmapfile, (skew/2), SEEK_CUR) != 0)
  {
    printf("\nError! File is incomplete.");
    return;
  }


  if(MVPD_DEBUG && (MVPD_DEBUG_LEVEL > 1))
  {
    printf("\n");
    printf("\nName:        %s" ,filename);
    printf("\nHeight:      %u pixel" ,bitmap->height);
    printf("\nWidth:       %u pixel" ,bitmap->width);
    printf("\nColordepth:  %u bit /pixel" ,colordepth);
    printf("\nSize:        %u Byte" ,size);
    printf("\nStartoffset: %u" ,offset);
    printf("\nSkew:        %u" ,skew);
  }

  for(i = matrix_y - 1; i > ((bitmap->height - 1) + output_y_skew) ; i--)
  {
    for(j = 0; j < matrix_x; j++)
    {
      for(a = 2; a >= 0; a--)
      {
        bitmap->picture[((i * matrix_x) + j) * 3 + a] = 0;
      }
    }
  }

  for(; i >= output_y_skew; i--)
  {
    for(j = 0; j < (output_x_skew/2); j++)
    {
      for(a = 2; a >= 0; a--)
      {
        bitmap->picture[((i * matrix_x) + j) * 3 + a] = 0;
      }
    }

    for(; j < (bitmap->width + (output_x_skew / 2)); j++)
    {
      for(a = 2; a >= 0; a--)
      {
        if(feof(bitmapfile))
        {
          printf("\nError! File is incomplete.");
          return;
        }
        temp = getc(bitmapfile);
        bitmap->picture[((i * matrix_x) + j) * 3 + a] = (mvpd_uint8) temp;
      }
    }

    for(; j < matrix_x; j++)
    {
      for(a = 2; a >= 0; a--)
      {
        bitmap->picture[((i * matrix_x) + j) * 3 + a] = 0;
      }
    }

    if(fseek(bitmapfile, skew, SEEK_CUR) != 0)
    {
      printf("\nError! File is incomplete.");
      return;
    }
    if(MVPD_DEBUG)
      printf(".");
  }

  for(; i >= 0 ; i--)
  {
    for(j = 0; j < matrix_x; j++)
    {
      for(a = 2; a >= 0; a--)
      {
        bitmap->picture[((i * matrix_x) + j) * 3 + a] = 0;
      }
    }
  }

  fclose(bitmapfile);

  free(char_buffer);
  free(int_buffer);

  if(MVPD_DEBUG)
    printf("\nLeaving bmp Constructor\n");
  return;
}

void MVPD_CLIP(mvpd_uint8* input, mvpd_uint16 input_x, mvpd_uint16 input_y, mvpd_uint8* output, mvpd_uint16 output_x, mvpd_uint16 output_y)
{
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nClipping started: ");
  fprintf(debug_log,"\nInput  X: %i  Y: %i", input_x, input_y);
  fprintf(debug_log,"\nOutput X: %i  Y: %i", output_x, output_y);
  fflush(debug_log);
#endif
  mvpd_uint16 input_skew;
  mvpd_uint16 output_skew;
  mvpd_uint32 max_x;

  mvpd_uint32 i,j;
  mvpd_uint32 input_counter = 0;
  mvpd_uint32 output_counter = 0;

  if(output_x > input_x)
  {
    output_skew = (output_x - input_x);
    input_skew = 0;
    max_x = input_x;
    output_counter = output_skew / 2;
  }
  else
  {
    input_skew = (input_x - output_x);
    output_skew = 0;
    max_x = output_x;
    input_counter = input_skew / 2;
  }

  if(input_y >output_y)
    input_counter += ((input_y - output_y ) * input_x) / 2;
  else
    output_counter += ((output_y - input_y ) * output_x) / 2;

  for(i = 0; ((i < input_y) && (i < output_y)); i++)
  {
    for(j = 0; j < max_x; j++)
      output[output_counter++] = input[input_counter++];
    output_counter += output_skew;
    input_counter += input_skew;
  }
#ifdef FUNCTION_DEBUG
  fprintf(debug_log,"\nClipping completed");
  fflush(debug_log);
#endif

}
