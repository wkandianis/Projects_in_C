#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "bitpack.h"
#include "except.h"
#include "assert.h"

/* 
 * What makes things hellish is that C does not define the effects of
 * a 64-bit shift on a 64-bit value, and the Intel hardware computes
 * shifts mod 64, so that a 64-bit shift has the same effect as a
 * 0-bit shift.  The obvious workaround is to define new shift functions
 * that can shift by 64 bits.
 */

Except_T Bitpack_Overflow = { "Overflow packing bits" };

static inline uint64_t shl(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word << bits;
}

/*
 * shift R logical
 */
static inline uint64_t shr(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                return 0;
        else
                return word >> bits;
}

/*
 * shift R arith
 */
static inline int64_t sra(uint64_t word, unsigned bits)
{
        assert(bits <= 64);
        if (bits == 64)
                bits = 63; /* will get all copies of sign bit, 
                            * which is correct for 64
                            */
	/* Warning: following uses non-portable >> on
	   signed value...see K&R 2nd edition page 206. */
        return ((int64_t) word) >> bits; 
}

/****************************************************************/
bool Bitpack_fitss( int64_t n, unsigned width)
{
        assert(width <= 64);
        int64_t narrow = sra(shl(n, 64 - width),
                             64 - width); 
        return narrow == n;
}

bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= 64);
        /* thanks to Jai Karve and John Bryan  */
        /* clever shortcut instead of 2 shifts */
        return shr(n, width) == 0; 
}

/****************************************************************/

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        if (width == 0) return 0;    /* avoid capturing unknown sign bit    */

        unsigned hi = lsb + width; /* one beyond the most significant bit */
        assert(hi <= 64);
        return sra(shl(word, 64 - hi),
                   64 - width);
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        assert(hi <= 64);
        /* different type of right shift */
        return shr(shl(word, 64 - hi),
                   64 - width); 
}

/****************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
        assert(width <= 64);
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        assert(hi <= 64);
        if (!Bitpack_fitsu(value, width))
                RAISE(Bitpack_Overflow);
        return shl(shr(word, hi), hi)                 /* high part */
                | shr(shl(word, 64 - lsb), 64 - lsb)  /* low part  */
                | (value << lsb);                     /* new part  */
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,
                      int64_t value)
{
        assert(width <= 64);
        if (!Bitpack_fitss(value, width))
                RAISE(Bitpack_Overflow);
        /* thanks to Michael Sackman and Gilad Gray */
        return Bitpack_newu(word, width, lsb, Bitpack_getu(value, width, 0));
}


/****************************************************************
 * undeclared exported test code                          
 *****************************************************************/
//#include "bitpack-tests.h"
// #include "fmt.h"
// 
// typedef void (*Bitpack_testfun)(const char *test,
//                                 Bitpack_flags flags,
//                                 bool passed,
//                                 void *cl);
// 
// void Bitpack_run_tests(bool print, Bitpack_testfun test, void *cl)
// {
// #define FLAG(P,F) ((P) ? (Bitpack_ ## F) : 0)
// #define FL(S64) (FLAG(S64, s64) | FLAG(w==0, w0) | FLAG(w==64, w64))
// #define CHECK(E, S64)                                                   \
//         TRY  test(#E, FL(S64), (E), cl);                                \
//         ELSE test(#E " (raised exception)", FL(S64), false, cl);        \
//         END_TRY
// #define NCHECK(E, SE, S64)                              \
//         TRY  test(Fmt_string SE, FL(S64), (E), cl);     \
//         ELSE test(Fmt_string SE, FL(S64), false, cl);   \
//         END_TRY
// #define FITS64 (w == 64) /* fitsx causes 64-bit shift */
// #define GET64  (w == 0)  /* getx  causes 64-bit shift */
// #define NEW64 (FITS64 || lsb == 0 || w + lsb == 64)
// #define NEW(E,F) TRY (void)(E);                                         \
//         ELSE test(#E " (raised exception)", FL(NEW64), false, cl);      \
//         goto F;                                                         \
//         END_TRY;                                                        \
//         uint64_t new = (E)
//         uint64_t words[]  = { 0U, ~0U, 0xfeedfacedeadbeef };
//         unsigned widths[] = {0, 6, 8, 64};  
//         unsigned lsbs[]   = {16, 0, 99};  /* 99 is placeholder for 64 - lsb */
//         /* all test values fit in 6 bits */
//         int64_t signs[]   = { 0xfffffffffffffffe, 7, 0, 11, -8, -11 };      
//         uint64_t unsigns[] = { 0x33, 7, 0, 11, 1 };      
// 
// /****************************************************************************/
// /* The following is terrible and may melt your eyes.  You have been warned! */
// /****************************************************************************/
// 
// #define NELEMS(A) (sizeof(A) / sizeof((A)[0])) /* standard macro */
//         /* test all combination of word, w, lsb, both signed and unsigned */
//         for (volatile unsigned i = 0; i < NELEMS(words); i++) {
//                 uint64_t old = words[i];
//                 for (volatile unsigned windex = 0;
//                      windex < NELEMS(widths);
//                      windex++) {
//                         unsigned w = widths[windex];
//                         for (volatile unsigned lindex = 0;
//                              lindex < NELEMS(lsbs);
//                              lindex++) {
//                                 volatile unsigned lsb
//                                         = (lsbs[lindex] == 99) ? 64 - w
//                                                                : lsbs[lindex];
// 
//                                 /* signed tests */
//                                 for (volatile unsigned j = 0;
//                                      j < NELEMS(signs);
//                                      j++) {
//                                         if ((signs[j] == 0 || w > 0)
//                                 && w + lsb <= 64) { /* sensible only */
//                         NEW(Bitpack_news(old, w, lsb,
//                                              signs[j]),
//                                     nexts);
//         \                    if (print) {
//                                 printf("-- news(0x%016"
//                                                        PRIx64
//                                                                ", %u, %u, %4"
//                                                            PRId64
//                                                                " (0x%04"
//                                                                PRIx64
//                                                                ") == 0x%016"
//                                                                PRIx64 "\n",
//                                                                old, w, lsb,
//                                                                signs[j],
//                                                                signs[j],
//                                                                new);
//                                                         printf("-- Recovered %"
//                                                                PRId64
//                                                                " from 0x%016"
//                                                                PRIx64
//                                                                " (was %"
//                                                                PRId64")\n",
//                                                                Bitpack_gets(new,
//                                                                             w,
//                                                                     lsb),
//                                                                new,
//                                                                signs[j]);
//                                                 }
//                 /* test old parts are unchanged and new part is signs[j] */
//             NCHECK(Bitpack_getu(new, lsb, 0) == Bitpack_getu(old, lsb, 0),
//                    ("Bitpack_getu(0x%W, %d, 0) == Bitpack_getu(0x%W, %d, 0)",
//                                             new, lsb, old, lsb),
//                                                        GET64||NEW64);
//                         NCHECK(Bitpack_getu(new, 64 - w - lsb, w + lsb) ==
//                                Bitpack_getu(old, 64 - w - lsb, w + lsb),
//            ("Bitpack_getu(0x%W, %d, %d) == Bitpack_getu(0x%W, %d, %d)",
//                     new, 64 - w - lsb, w + lsb, old, 64 - w - lsb, w + lsb),
//                                    GET64||NEW64);
//                         NCHECK(Bitpack_gets(new, w, lsb) == signs[j],
//              ("Bitpack_gets(0x%W, %d, %d) == %d", new, w, lsb, signs[j]),
//                                            GET64||NEW64);
//                                         nexts: (void)0;
//                                         }
//                                 }
// 
//                 /* unsigned tests */
//                 for (volatile unsigned j = 0; j < NELEMS(unsigns); j++) {
//                         if ((unsigns[j] == 0 || w > 0) && w + lsb <= 64) {
//                     NEW(Bitpack_newu(old, w, lsb, unsigns[j]), nextu);
//                                                 if (print) {
//                         printf("-- newu(0x%016" PRIx64 ", %u, %u, %4" PRIu64
//                                    " (0x%04" PRIx64 ") == 0x%016" PRIx64 "\n",
//                                old, w, lsb, unsigns[j], unsigns[j], new);
//                                                 }
//             /* test old parts are unchanged and new part is unsigns[j] */
//             NCHECK(Bitpack_getu(new, lsb, 0) == Bitpack_getu(old, lsb, 0),
//                    ("Bitpack_getu(0x%W, %d, 0) == Bitpack_getu(0x%W, %d, 0)",
//                         new, lsb, old, lsb),
//                                                        GET64||NEW64);
//                 NCHECK(Bitpack_getu(new, 64 - w - lsb, w + lsb) ==
//                            Bitpack_getu(old, 64 - w - lsb, w + lsb),
//            ("Bitpack_getu(0x%W, %d, %d) == Bitpack_getu(0x%W, %d, %d)",
//                         new, 64 - w - lsb, w + lsb, old, 64 - w - lsb, w + lsb),
//                                                        GET64||NEW64);
//             NCHECK(Bitpack_getu(new, w, lsb) == unsigns[j],
//                ("Bitpack_gets(0x%W, %d, %d) == %u", new, w, lsb, unsigns[j]),
//                                                        GET64||NEW64);
//                                         nextu: (void)0;
//                                         }
//                                 }
//                         }
//                 }
//         }
// }



// /*                                                                           
// *     bitpack.c                                                           
// *     by Cate Desler (cdesle01) and Will Kandianis (wkandi01), 3/6/20     
// *     hw4                                                                    
// *                                                                              
// *     description: this is the implementation of bitpack.h containing all the 
// *                  functions described 
// */
// 
// #include "bitpack.h"
// #include "math.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
// 
// uint64_t clearField(uint64_t word, unsigned lsb, unsigned width);
// int64_t shiftRightS(int64_t word, unsigned offset, unsigned width);
// uint64_t shiftRight(uint64_t word, unsigned shift);
// uint64_t shiftLeft(uint64_t word, unsigned shift);
// 
// /*  Function - shiftRight
//  *  Arguments: a uint64_t representing the bits being manipulated and an
//  *             unsigned representing the amount the uint64_t is being shifted
//  *             to the right
//  *  Return: a uint64_t representing the shifted bits
//  *  Purpose: shift the given uint64_t by the given shift to the right
//  */
// uint64_t shiftRight(uint64_t word, unsigned shift)
// {
//     if (shift == 64) {
//         return 0;
//     }
//     else {
//         word = word >> shift;
//     }
//     return word;
// }
// 
// /*  Function - shiftLeft
//  *  Arguments: a uint64_t representing the bits being manipulated and an
//  *             unsigned representing the amount the uint64_t is being shifted
//  *             to the left
//  *  Return: a uint64_t representing the shifted bits
//  *  Purpose: shift the given uint64_t by the given shift to the left
//  *   
//  */
// uint64_t shiftLeft(uint64_t word, unsigned shift)
// {
//     assert(shift <= 64);
//     if (shift == 64) {
//         return 0;
//     }
//     else {
//         word = word << shift;
//     }
//     return word;
// }
// 
// /*  Function - shiftRightS
//  *  Arguments: a int64_t representing the bits being manipulated, an unsigned
//  *             representing the amount the uint64_t is being shifted to the
//  *             right, and 
//  *  Return: the newly shifted word
//  *  Purpose: to shift a word to the right, and populate it with 1s because
//  *           this function is for signed values
//  *   
//  */
// int64_t shiftRightS(int64_t word, unsigned shift, unsigned width)
// {
//     assert(width <= 64);
//     assert(shift <= 64);
//     if (shift == 64){
//             return 0;
//     }
//     uint64_t mask;
//     word = shiftRight(word, shift);
//     uint64_t check = ~0;
//     check = shiftLeft(check, 63);
//     check = shiftRight(check, 64 - width);
//     check = (check & word);
//     check = shiftRight(check, width - 1);
//      if (check == 1) {
//             mask = ~0;
//             mask = shiftLeft(mask, width);
//             word = (word | mask);
//     }    
//     return word;
// }
// 
// /*  Function - Bitpack_fitsu
//  * Arguments: a uint64_t n, and an unsigned width
//  *    Return: boolean value representing if n can fit in width bits
//  *   Purpose: To determine if n can be represented in width bits
//  *   
//  */
// bool Bitpack_fitsu(uint64_t n, unsigned width)
// {
//     assert(width <= 64);
//     uint64_t check = 1; 
//     check = shiftLeft(check, width);
//     check--;
//     if (n <= check) {
//             return true;
//     }
//     return false;
// }
// 
// /*  Function - Bitpack_fitss
//  * Arguments: a uint64_t n, and an unsigned width
//  *    Return: boolean value representing if n can fit in width bits
//  *   Purpose: To determine if n can be represented in width bits
//  *            in a signed value
//  *   
//  */
// bool Bitpack_fitss(int64_t n, unsigned width)
// {
//     assert(width <= 64);
//     int64_t check = ~0;
//     check = shiftRightS(check, width - 1, 0);
//     check = shiftLeft(check, width - 1);
//     if ((n <= ~check) && (n >= check)) {
//             return true;
//     }
//     return false;
// }
// 
// /*  Function - Bitpack_gets
//  *  Arguments: a uint64_t representing the bits to be searched, and two 
//  *             unsigneds representing the width and the least significant bit 
//  *             in the field 
//  *  Return:   the newly modified word
//  *  Purpose:  to extract a certain field from a word
//  *   
//  */
// uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
// {
//     assert(width <= 64);
//     assert(lsb <= 64);
//     uint64_t mask = ~0;
//     uint64_t newword;
//     mask = shiftRight(mask, (64 - width));
//     mask = shiftLeft(mask, lsb);
//     newword = word & mask;
//     newword = shiftRight(newword, lsb);
//     return newword;
// }
// 
// /*  Function - Bitpack_gets
//  *  Arguments: a uint64_t representing the bits to be searched, and two 
//  *             unsigneds representing the width and the least significant bit 
//  *             in the field 
//  *  Return:   the newly modified word
//  *  Purpose:  to extract a certain field from a signed word
//  *   
//  */
// int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
// {
//     assert(width <= 64);
//     assert(lsb <= 64);
//     uint64_t mask = ~0;
//     int64_t newword;
//     mask = shiftRight(mask, (64 - width));
//     mask = shiftLeft(mask, lsb);
//     newword = word & mask;
//     newword = shiftRightS(newword, lsb, width);
//     return newword;
// }
// 
// /*  Function - Bitpack_newu
//  *  Arguments: a uint64_t representing the bits to be modified, two unsigneds 
//  *             representing the width and the least significant bit of the field
//  *             and an uint64_t representing the value to be put into the field 
//  *  Return: a uint64_t representing the updated word
//  *  Purpose: to replace a field of a word with the given value
//  *   
//  */
// uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
//                       uint64_t value)
// {
//     assert(width <= 64);
//     assert((width + lsb) <= 64);
//     assert(lsb <= 64);  
//     assert(Bitpack_fitsu(value, width));  
//     uint64_t mask = 0;
//     uint64_t newword = 0;
//     word = clearField(word, lsb, width);
//     mask = value << lsb;
//     newword = (word | mask);
//     return newword;
// }
// 
// /*  Function - Bitpack_news
//  *  Arguments: a uint64_t representing the bits to be modified, two unsigneds 
//  *             representing the width and the least significant bit of the field
//  *             and an int64_t representing the value to be put into the field 
//  *  Return: a uint64_t representing the updated word
//  *  Purpose: to replace a field of a word with the given signed value
//  *   
//  */
// uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
//                                                         int64_t value)
// {   
//     assert(width <= 64);
//     assert((width + lsb) <= 64);
//     assert(lsb <= 64);    
//     assert(Bitpack_fitss(value, width));
//     word = clearField(word, lsb, width);    
//     uint64_t mask = ~0;
//     mask = mask << (64 - width);
//     mask = mask >> (64 - (width + lsb));
//     value = value << lsb;
//     mask = (value & mask);
//     word = (mask | word);
//     return word;
// }
// 
// /*  Function - clearField
//  *  Arguments: a uint64_t representing the bits being manipulated, and two 
//  *             representing the least significant bit and the width of the field
//  *  Return: a uint64_t representing the modified word
//  *  Purpose: to clear the given filed in the given word by making all bits 
//  *           equal to 0
//  *   
//  */
// uint64_t clearField(uint64_t word, unsigned lsb, unsigned width)
// {
//     assert(width <= 64);
//     assert(lsb <= 64);
//     uint64_t mask = ~0;
//     mask = mask >> (64 - width);
//     mask = mask << lsb;
//     mask = ~mask;
//     word = (word & mask);
//     return word;
// }