/*
 * Problem 5.6
 * Assume: In MTX, P1 is a Casanova process, which hops, not from bed to bed,
 * but from segment to segment, Initially, P1 runs in the segment 0x2000.
 * By a hop(u16 segment) syscall, it enters kernel to change segment. When it
 * returns to Umode, it returns to an identical Umode image but in a different
 * segment, e.g. 0x4000. Devise an algorithm for the hop() syscall, and implment
 * it in MTX to satisfy the lusts of Casnova process.
 */

int hop(u16 segment)
{
    //printf("In HOP\n");
    //printf("%x\n", segment);
    copyImage(running->uss, segment, 16*1024); // copy 16K words
    running->uss = segment;                          // child’s own segment
    //running->usp = running->usp;                     // same as parent's usp
    //printf("running->usp = %d\n", running->usp);
    //*** change uDS, uES, uCS, AX in child's ustack ****
    put_word(segment, segment, running->usp);        // uDS=segment
    put_word(segment, segment, running->usp+2);      // uES=segment
    put_word(segment, segment, running->usp+2*8);          // uax=0
    put_word(segment, segment, running->usp+2*10);   // uCS=segment
    return running->uss;
}
