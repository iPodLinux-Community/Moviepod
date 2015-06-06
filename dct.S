
#ifdef IPOD
	
	.section .text
	
__ASM_functions_list:
	.long	_MVPD_ARM_COLUMN_DCT
	.long	_MVPD_ARM_ROW_DCT
	.long	0

  /***************************************************************
   * Warning the following code is scary and highly confusing!   *
   * Also it probably is really inefficent.                      *
   * Keep out of reach of small children, cute furry animals and *
   * visual basic programmers.                                   *
   ***************************************************************/

  /* This code calculates the inverse discrete cosine of a 8 x 8 *
   * Block. The code calculates first the even part, then stores *
   * the resulting four 16 Bit values in two 32 Bit registers.   *
   * Then it calculates the odd part.                            *
   * The loop has been unrolled, to get one more register.       */

	.globl	_MVPD_ARM_COLUMN_DCT
_MVPD_ARM_COLUMN_DCT:
	
	stmdb	sp!, {r3-r12, lr}
	mov	r11, r0			@ r11 = input
	mov	r12, r1			@ r12 = temp
	mov	r14, #8			@ loop eight times
	


      /*Odd Part                                              *
       *Variables:                                            *
       * r11, r12 - input and temp pointers                   *
       *                                                      *
       * r14 - is the loop counter                            *
       *                                                      *
       * r9, r10 - will be used for temporary calculations    *
       *           and to store the final results             *
       * r8 - will be used as a temporary variable for        *
       *      multiplication                                  *
       *                                                      *
       * r0  - equals z10 in the original implementation      *
       * r1  - equals z11 in the original implementation      *
       * r2  - equals z12 in the original implementation      *
       * r3  - equals z13 in the original implementation      *
       *                                                      *
       * r4  - equals tmp4 in the original implementation     *
       * r5  - equals tmp5 in the original implementation     *
       * r6  - equals tmp6 in the original implementation     *
       * r7  - equals tmp7 in the original implementation     *
       *                                                      */

1:
	ldrsh r0, [r11], #16	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r4, [r11], #16	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r1, [r11], #16	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r5, [r11], #16	@@ Load second value, then increment by 32 to point to the next odd line
	ldrsh r2, [r11], #16	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r6, [r11], #16	@@ Load third  value, then increment by 32 to point to the next odd line
	ldrsh r3, [r11], #16	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r7, [r11], #16	@@ Load fourth value, then increment by 32 to point to the next odd line

	cmp r4, #0
	bne 2f
	cmp r1, #0
	bne 2f
	cmp r5, #0
	bne 2f
	cmp r2, #0
	bne 2f
	cmp r6, #0
	bne 2f
	cmp r3, #0
	bne 2f
	cmp r7, #0
	bne 2f
	
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	strh r0, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	sub r12, r12, #126	@@ Return temp pointer to the first element in the next collumn
	sub r11, r11, #126	@@ Return input pointer to the second element in the next collumn
	
	b 3f

2:
	add r3, r6, r5		@@ z13 = tmp6 + tmp5
	sub r0, r6, r5		@@ z10 = tmp6 - tmp5
	add r1, r4, r7		@@ z11 = tmp4 + tmp7
	sub r2, r4, r7		@@ z12 = tmp4 - tmp7

	add r7, r1, r3		@@ tmp7 = z11 + z13
	
	ldr r8, =0x000001D9	@@ r8 = 0x01D9 = 473 =~ 1.84 * 256
	add r5, r0, r2		@@ r5 is used as a temporary variable to calculate z5 = z10 + z12
	mul r5, r5, r8		@@ z5 = z5 * 473, lets just hope it still fits. *
	mov r5, r5, asr #8	@@ z5 = z5 * 473 / 256 = z5 * 1.84

	ldr r8, =0xFFFFFD63	@@ r8 = 0xFD63 = -669 =~ -2.61 * 256
	mul r6, r0, r8		@@ r6 equals tmp12 in this part tmp12 =  z10 * -2.61

	add r6, r5, r6, asr #8	@@ tmp12 = (tmp12 >> 8) + z5
	
	sub r6, r6, r7		@@ tmp6 = tmp12 - tmp7

	ldr r8, =0x00000115	@@ r8 = 0x0115 = 277 =~ 1.08 * 256
	mul r0, r2, r8		@@ r0 equals tmp10 in this part, tmp10 =  z12 * 1.08
	rsb r0, r5, r0, asr #8	@@ tmp10 = (tmp10 >> 8) - z5

	ldr r8, =0x0000016A	@@ r8 = 0x016A = 362 =~ 1.41 * 256

	sub r2, r1, r3		@@ tmp11 = z11 - z13
	mul r5, r2, r8		@@ r5 equals tmp11 in this part, tmp11 = (z11 - z13) * 1.41
	rsb r5, r6, r5, asr #8	@@ tmp5 = (tmp11 >> 8) - tmp6

	add r4, r0, r5		@@ tmp4 = tmp10 + tmp5

      /* Now the values of tmp4 and tmp5 are stored in r9   *
       * And the values of tmp6 and tmp7 are stored in r10  *
       * This is done without checking wether those values  *
       * are still in the 16 Bit range, i just hope they    *
       * are.                                               *
       * This leaves the registers r0 - r7 free for the     *
       * calculation of the even values                     */

       /* Note: this should also be doable with a pack      *
        * instruction.                                      */

      /* Just pray that these values are not bigger than 0xFFFF*/
	ldr	r8, =0x0000FFFF	@@ A Mask to cut everything down to 16 Bit
	and r4, r4, r8
	and r5, r5, r8
	and r6, r6, r8
	and r7, r7, r8
	
	mov r9, r4, lsl #16	@@ Load tmp4 in r9[high]
	orr r9, r9, r5		@@ Load tmp5 in r9[low]
	mov r10, r6, lsl #16	@@ Load tmp6 in r10[high]
	orr r10, r10, r7	@@ Load tmp7 in r10[low]
	
      /*Even Part                                              *
       *Variables:                                            *
       * r11, r12 - input and temp pointers                   *
       *                                                      *
       * r9, r10 - hold the odd values                        *
       * r8 - will be used as a temporary variable for        *
       *      multiplication                                  *
       *                                                      *
       * r0  - equals tmp0 in the original implementation     *
       * r1  - equals tmp1 in the original implementation     *
       * r2  - equals tmp2 in the original implementation     *
       * r3  - equals tmp3 in the original implementation     *
       *                                                      *
       * r4  - equals tmp10 in the original implementation    *
       * r5  - equals tmp11 in the original implementation    *
       * r6  - equals tmp12 in the original implementation    *
       * r7  - equals tmp13 in the original implementation    *
       *                                                      */
	sub r11, r11, #128	@@ Return input pointer to the first element in the next column
	ldrsh r0, [r11], #32	@@ Load first  value, then increment by 32 to point to the next even line */
	ldrsh r1, [r11], #32	@@ Load second  value, then increment by 32 to point to the next even line */
	ldrsh r2, [r11], #32	@@ Load third  value, then increment by 32 to point to the next even line */
	ldrsh r3, [r11], #32	@@ Load fourth  value, then increment by 32 to point to the next even line */

	add r4, r0, r2		@@ tmp10 = tmp0 + tmp2
	sub r5, r0, r2		@@ tmp11 = tmp0 - tmp2
	add r7, r1, r3		@@ tmp13 = tmp1 + tmp3
	sub r6, r1, r3		@@ tmp12 = tmp1 - tmp3
	
	ldr r8, =0x0000016A	@@ r8 = 0x016A = 362 =~ 1.41 * 256
	mul r6, r6, r8		@@ tmp12 = tmp12 * 362, lets just hope it still fits.*/
	rsb r6, r7, r6, asr #8	@@ tmp12 = (tmp12 >> 8) - tmp13 */

	add r0, r4, r7		@@ tmp0 = tmp10 + tmp13 */
	sub r3, r4, r7		@@ tmp3 = tmp10 - tmp13 */
	add r1, r5, r6		@@ tmp1 = tmp11 + tmp12 */
	sub r2, r5, r6		@@ tmp2 = tmp11 - tmp12 */

      /* Now we have to reload the values of tmp4 - tmp7 from *
       * r9 and r10 into r4 - r7. This will give us the       *
       * Variables tmp0 - tmp7 in the registers r0 -r7.       *
       * The registers r8, r9 and r10 are then left avaible   *
       * for temporary values, but only r8 is needed.         */
	
	ldr	r8, =0x0000FFFF	@@ A Mask to cut everything down to 16 Bit
	mov r4, r9, asr #16	@@ tmp4 is r9[high]
	and r5, r9, r8		@@ tmp5 is r9[low]
	mov r6, r10, asr #16	@@ tmp4 is r9[high]
	and r7, r10, r8		@@ tmp5 is r9[low]
	
/*
	mov r5, r5, lsl #16
	mov r5, r5, asr #16
	mov r7, r7, lsl #16
	mov r7, r7, asr #16
*/

	add r8, r0, r7		@@ r8 = tmp0 + tmp7
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	add r8, r1, r6		@@ r8 = tmp1 + tmp6
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	add r8, r2, r5		@@ r8 = tmp2 + tmp5
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	sub r8, r3, r4		@@ r8 = tmp3 - tmp4
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	add r8, r3, r4		@@ r8 = tmp3 + tmp4
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	sub r8, r2, r5		@@ r8 = tmp2 - tmp5
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	sub r8, r1, r6		@@ r8 = tmp1 - tmp6
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	sub r8, r0, r7		@@ r8 = tmp0 - tmp7
	strh r8, [r12], #16	@@ Store first  value, then increment by 16 to point to the next line
	
	sub r12, r12, #126	@@ Return temp pointer to the first element in the next collumn
	sub r11, r11, #126	@@ Return input pointer to the second element in the next collumn

3:
	subs r14, r14, #1	
	bne 1b



	
	
	ldmia	sp!, {r3-r12, pc}
	.pool

	


	.globl	_MVPD_ARM_ROW_DCT
_MVPD_ARM_ROW_DCT:
	
	stmdb	sp!, {r3-r12, lr}
	mov	r11, r0			@@ r11 = input
	mov	r12, r1			@@ r12 = temp
	mov	r14, #8			@@ loop 8 times

      /*Odd Part                                              *
       *Variables:                                            *
       * r11, r12 - input and temp pointers                   *
       *                                                      *
       * r14 - loop counter                                   *
       *                                                      *
       * r9, r10 - will be used for temporary calculations    *
       *           and to store the final results             *
       * r8 - will be used as a temporary variable for        *
       *      multiplication                                  *
       *                                                      *
       * r0  - equals z10 in the original implementation      *
       * r1  - equals z11 in the original implementation      *
       * r2  - equals z12 in the original implementation      *
       * r3  - equals z13 in the original implementation      *
       *                                                      *
       * r4  - equals tmp4 in the original implementation     *
       * r5  - equals tmp5 in the original implementation     *
       * r6  - equals tmp6 in the original implementation     *
       * r7  - equals tmp7 in the original implementation     *
       *                                                      */
1:
	ldrsh r0, [r11], #2	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r4, [r11], #2	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r1, [r11], #2	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r5, [r11], #2	@@ Load second value, then increment by 32 to point to the next odd line
	ldrsh r2, [r11], #2	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r6, [r11], #2	@@ Load third  value, then increment by 32 to point to the next odd line
	ldrsh r3, [r11], #2	@@ Load first  value, then increment by 32 to point to the next odd line
	ldrsh r7, [r11], #2	@@ Load fourth value, then increment by 32 to point to the next odd line

	cmp r4, #0
	bne 2f
	cmp r1, #0
	bne 2f
	cmp r5, #0
	bne 2f
	cmp r2, #0
	bne 2f
	cmp r6, #0
	bne 2f
	cmp r3, #0
	bne 2f
	cmp r7, #0
	bne 2f

	mov r9, #0x80		@@ 128 to increment
	mov r10, #0xFF		@@ to cut everything down to 8 Bit
	
	add r8, r9, r0, asr #3	@@ 
	and r8, r8, r10 

	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line
	b 3f


2:
	add r3, r6, r5		@@ z13 = tmp6 + tmp5
	sub r0, r6, r5		@@ z10 = tmp6 - tmp5
	add r1, r4, r7		@@ z11 = tmp4 + tmp7
	sub r2, r4, r7		@@ z12 = tmp4 - tmp7

	add r7, r1, r3		@@ tmp7 = z11 + z13
	
	ldr r8, =0x000001D9	@@ r8 = 0x01D9 = 473 =~ 1.84 * 256
	add r5, r0, r2		@@ r5 is used as a temporary variable to calculate z5 = z10 + z12
	mul r5, r5, r8		@@ z5 = z5 * 473, lets just hope it still fits. *
	mov r5, r5, asr #8	@@ z5 = z5 * 473 / 256 = z5 * 1.84
	
	ldr r8, =0xFFFFFD63	@@ r8 = 0xFD63 = -669 =~ -2.61 * 256
	mul r6, r0, r8		@@ r6 equals tmp12 in this part tmp12 =  z10 * -2.61
	add r6, r5, r6, asr #8	@@ tmp12 = (tmp12 >> 8) + z5
	
	sub r6, r6, r7		@@ tmp6 = tmp12 - tmp7

	ldr r8, =0x00000115	@@ r8 = 0x0115 = 277 =~ 1.08 * 256
	mul r0, r2, r8		@@ r0 equals tmp10 in this part, tmp10 =  z12 * 1.08
	rsb r0, r5, r0, asr #8	@@ tmp10 = (tmp10 >> 8) - z5

	ldr r8, =0x0000016A	@@ r8 = 0x016A = 362 =~ 1.41 * 256

	sub r2, r1, r3		@@ tmp11 = z11 - z13
	mul r5, r2, r8		@@ r5 equals tmp11 in this part, tmp11 = (z11 - z13) * 1.41
	rsb r5, r6, r5, asr #8	@@ tmp5 = (tmp11 >> 8) - tmp6

	add r4, r0, r5		@@ tmp4 = tmp10 + tmp5
	
      /* Now the values of tmp4 and tmp5 are stored in r9   *
       * And the values of tmp6 and tmp7 are stored in r10  *
       * This is done without checking wether those values  *
       * are still in the 16 Bit range, i just hope they    *
       * are.                                               *
       * This leaves the registers r0 - r7 free for the     *
       * calculation of the even values                     */

       /* Note: this should also be doable with a pack      *
        * instruction.                                      */

      /* Just pray that these values are not bigger than 0xFFFF*/
	ldr	r8, =0x0000FFFF	@@ A Mask to cut everything down to 16 Bit
	and r4, r4, r8
	and r5, r5, r8
	and r6, r6, r8
	and r7, r7, r8
	
	mov r9, r4, lsl #16	@@ Load tmp4 in r9[high]
	add r9, r9, r5		@@ Load tmp5 in r9[low]
	mov r10, r6, lsl #16	@@ Load tmp6 in r10[high]
	add r10, r10, r7	@@ Load tmp7 in r10[low]
	
      /*Even Part                                              *
       *Variables:                                            *
       * r11, r12 - input and temp pointers                   *
       *                                                      *
       * r9, r10 - hold the odd values                        *
       * r8 - will be used as a temporary variable for        *
       *      multiplication                                  *
       *                                                      *
       * r0  - equals tmp0 in the original implementation     *
       * r1  - equals tmp1 in the original implementation     *
       * r2  - equals tmp2 in the original implementation     *
       * r3  - equals tmp3 in the original implementation     *
       *                                                      *
       * r4  - equals tmp10 in the original implementation    *
       * r5  - equals tmp11 in the original implementation    *
       * r6  - equals tmp12 in the original implementation    *
       * r7  - equals tmp13 in the original implementation    *
       *                                                      */
	sub r11, r11, #16	@@ Return input pointer to the first element in the next column
	ldrsh r0, [r11], #4	@@ Load first  value, then increment by 32 to point to the next even value
	ldrsh r1, [r11], #4	@@ Load second  value, then increment by 32 to point to the next even value
	ldrsh r2, [r11], #4	@@ Load third  value, then increment by 32 to point to the next even value
	ldrsh r3, [r11], #4	@@ Load fourth  value, then increment by 32 to point to the next even value

	add r4, r0, r2		@@ tmp10 = tmp0 + tmp2
	sub r5, r0, r2		@@ tmp11 = tmp0 - tmp2
	add r7, r1, r3		@@ tmp13 = tmp1 + tmp3
	sub r6, r1, r3		@@ tmp12 = tmp1 - tmp3
	
	ldr r8, =0x0000016A	@@ r8 = 0x016A = 362 =~ 1.41 * 256
	mul r6, r6, r8		@@ tmp12 = tmp12 * 362, lets just hope it still fits.*/
	rsb r6, r7, r6, asr #8	@@ tmp12 = (tmp12 >> 8) - tmp13 */

	add r0, r4, r7		@@ tmp0 = tmp10 + tmp13 */
	sub r3, r4, r7		@@ tmp3 = tmp10 - tmp13 */
	add r1, r5, r6		@@ tmp1 = tmp11 + tmp12 */
	sub r2, r5, r6		@@ tmp2 = tmp11 - tmp12 */

      /* Now we have to reload the values of tmp4 - tmp7 from *
       * r9 and r10 into r4 - r7. This will give us the       *
       * Variables tmp0 - tmp7 in the registers r0 -r7.       *
       * The registers r8, r9 and r10 are then left avaible   *
       * for temporary values, but only r8 is needed.         */
	
	ldr r8, =0x0000FFFF	@@ A Mask to cut everything down to 16 Bit
	mov r4, r9, asr #16	@@ tmp4 is r9[high]
	and r5, r9, r8		@@ tmp5 is r9[low]
	mov r6, r10, asr #16	@@ tmp6 is r10[high]
	and r7, r10, r8		@@ tmp7 is r10[low]
	
	mov r5, r5, lsl #16
	mov r5, r5, asr #16
	mov r7, r7, lsl #16
	mov r7, r7, asr #16

	
      /*Store the results in the output  */
	mov r9, #0x80		@@ 128 to increment
	mov r10, #0xFF		@@ to cut everything down to 8 Bit
	
	add r8, r0, r7		@@ r8 = tmp0 + tmp7 */
	add r8, r9, r8, asr #3	@@
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	add r8, r1, r6		@@ r8 = tmp1 + tmp6 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	add r8, r2, r5		@@ r8 = tmp2 + tmp5 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	sub r8, r3, r4		@@ r8 = tmp3 - tmp4 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	add r8, r3, r4		@@ r8 = tmp3 + tmp4 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	sub r8, r2, r5		@@ r8 = tmp2 - tmp5 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	sub r8, r1, r6		@@ r8 = tmp1 - tmp6 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
	sub r8, r0, r7		@@ r8 = tmp0 - tmp7 */
	add r8, r9, r8, asr #3	@@ 
	and r8, r8, r10 
	strh r8, [r12], #2	@@ Store first  value, then increment by 16 to point to the next line */
	
3:
	subs r14, r14, #1	
	bne 1b

	ldmia	sp!, {r3-r12, pc}
	.pool



__ASM_functions_end:
	
	.section .data
need_finishup:	.long 0
	
@ Local Variables:
@ asm-comment-char: ?@
@ comment-start: "@ "
@ block-comment-start: "/*"
@ block-comment-end: "*/"
@ indent-tabs-mode: t
@ End:

#endif
