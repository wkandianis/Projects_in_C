/*                                                                           
*     CosineandWord.c                                                             
*     by Cate Desler (cdesle01) and Will Kandianis (wkandi01), 3/6/20     
*     hw4                                                                    
*                                                                              
*     description: the implementation of CosineandWord.h containing all of the 
*                  functions implemented
*/

#include <stdio.h>
#include <stdlib.h>
#include "CVandCosine.h"
#include "uarray.h"
#include "seq.h"
#include "assert.h"
#include "bitpack.h"

const int A_LSB = 26; 
const int B_LSB = 20;
const int C_LSB = 14;
const int D_LSB = 8;
const int APB_LSB = 4;
const int APR_LSB = 0;
const int A_WIDTH = 6;
const int BCD_WIDTH = 6;
const int APB_APR_WIDTH = 4;
const int BYTE_1 = 0;
const int BYTE_2 = 8;
const int BYTE_3 = 16;
const int BYTE_4 = 24;
const int BYTE_SIZE = 8;

void print_word(uint32_t word);

/*  Function - CosinetoWord 
 *  Arguments: A cosine struct
 *  Return: uint32_t
 *  Purpose: Takes in a cosine struct, retrieves the discrete Cosine
 *           values from the struct, and packs them into a word that is 
 *           returned.
 */
uint32_t CosinetoWord(struct Cosine *CosineValues) 
{
    assert(CosineValues);
    uint32_t word = 0;
    unsigned a = CosineValues->a;
    int b = CosineValues->b;
    int c = CosineValues->c;
    int d = CosineValues->d;
    unsigned APb = CosineValues->APb;
    unsigned APr = CosineValues->APr;
    word = Bitpack_newu(word, A_WIDTH, A_LSB, a);
    word = Bitpack_news(word, BCD_WIDTH, B_LSB, b);
    word = Bitpack_news(word, BCD_WIDTH, C_LSB, c);
    word = Bitpack_news(word, BCD_WIDTH, D_LSB, d);
    word = Bitpack_newu(word, APB_APR_WIDTH, APB_LSB, APb);
    word = Bitpack_newu(word, APB_APR_WIDTH, APR_LSB, APr);
    return word;
}

/*  Function - CosineSeqtoWord 
 *  Arguments: A sequence of cosine structs, width, height
 *  Purpose: To remove the last element in the sequence, and pass that 
 *           element to CosinetoWord. Function then calls print word and frees
 *           the element removed from the sequence
 */
void CosineSeqtoWord(Seq_T CosineSeq)
{
    assert(CosineSeq);
    while(Seq_length(CosineSeq) > 0) {
        struct Cosine *toConvert = (struct Cosine *) Seq_remlo(CosineSeq);
        uint32_t word = CosinetoWord(toConvert);
        print_word(word);
        free(toConvert);
    }
}

/*  Function - WordtoCosine 
 *  Arguments: A uint32_t word
 *  Return: Cosine struct
 *  Purpose: Gets specific fields of the word and assigns them to the 
 *           values of the cosine struct. Then returns said struct
 */
struct Cosine * WordtoCosine(uint32_t word)
{    
    struct Cosine *CosineValues = malloc(sizeof(struct Cosine));
    CosineValues->a   = Bitpack_getu(word, A_WIDTH, A_LSB);
    CosineValues->b   = Bitpack_gets(word, BCD_WIDTH, B_LSB);
    CosineValues->c   = Bitpack_gets(word, BCD_WIDTH, C_LSB);
    CosineValues->d   = Bitpack_gets(word, BCD_WIDTH, D_LSB);
    CosineValues->APb = Bitpack_getu(word, APB_APR_WIDTH, APB_LSB);
    CosineValues->APr = Bitpack_getu(word, APB_APR_WIDTH, APR_LSB);
    return CosineValues;
}

/*  Function - WordstoCosineSeq 
 *  Arguments: A UArray2 of words, width, height 
 *  Return: A sequence of cosine structs
 *  Purpose: Iterates through the Uarray2 of words, and calls 
 *           WordtoCosine on each word in the array. Then adds the Cosine
 *           struct returned from WordtoCosine onto the sequence
 */
Seq_T WordstoCosineSeq(UArray2_T WordArray, unsigned width, unsigned height) 
{
    Seq_T CosineSeq = Seq_new((width * height)/4);
    unsigned i, j;
    for (i = 0; i < width/2; i++) {
        for (j = 0; j < height/2; j++) {
            uint32_t word = *(uint32_t *)UArray2_at(WordArray, i, j);
            struct Cosine *CosineValues = WordtoCosine(word);
            Seq_addhi(CosineSeq, CosineValues);
        }
    }
    return CosineSeq;
}

/*  Function - print_word 
 *  Arguments: A uint32_t word, width, height
 *  Purpose: To retreive each byte from the given word and print them 
 *           in big endian order
 */
void print_word(uint32_t word)
{
    char toPrint1 = (char) Bitpack_getu(word, BYTE_SIZE, BYTE_1);
    char toPrint2 = (char) Bitpack_getu(word, BYTE_SIZE, BYTE_2);
    char toPrint3 = (char) Bitpack_getu(word, BYTE_SIZE, BYTE_3);
    char toPrint4 = (char) Bitpack_getu(word, BYTE_SIZE, BYTE_4);
    putc(toPrint4, stdout);
    putc(toPrint3, stdout);
    putc(toPrint2, stdout);
    putc(toPrint1, stdout);
}

/*  Function - read_word
 *  Arguments: A file pointer, width, height
 *  Return: UArray2_T of words
 *  Purpose: Takes in a cosine struct, retrieves the discrete Cosine
 *           values from the struct, and packs them into a word that is 
 *           returned.
 */
UArray2_T read_word(FILE *fp, unsigned width, unsigned height)
{
    if (fp == NULL) {
        fprintf(stderr, "%s\n", "Compressed image file could not be opened");
        exit(EXIT_FAILURE);
    }
        unsigned i, j;
        UArray2_T wordArr = UArray2_new(width/2, height/2, sizeof(uint32_t));
        for (i = 0; i < width/2; i++) {
            for (j = 0; j < height/2; j++) {
                char c1 = getc(fp);
                char c2 = getc(fp);
                char c3 = getc(fp);
                char c4 = getc(fp);
                uint32_t word;
                if (c1 > 0) {
                    word = Bitpack_newu(word, BYTE_SIZE, BYTE_4, c1);
                } else {
                    word = Bitpack_news(word, BYTE_SIZE, BYTE_4, c1);
                }
                if (c2 > 0) {
                    word = Bitpack_newu(word, BYTE_SIZE, BYTE_3, c2);
                } else {
                    word = Bitpack_news(word, BYTE_SIZE, BYTE_3, c2);
                }
                if (c3 > 0) {
                    word = Bitpack_newu(word, BYTE_SIZE, BYTE_2, c3);
                } else {
                    word = Bitpack_news(word, BYTE_SIZE, BYTE_2, c3);
                }
                if (c4 > 0) {
                    word = Bitpack_newu(word, BYTE_SIZE, BYTE_1, c4);
                } else {
                    word = Bitpack_news(word, BYTE_SIZE, BYTE_1, c4);
                }
                uint32_t *elem = (uint32_t *) UArray2_at(wordArr, i, j);
                *elem = word;
        }
    }
    return wordArr;
}