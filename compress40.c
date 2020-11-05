/*                                                                           
*     compress40.c                                                             
*     by Cate Desler (cdesle01) and Will Kandianis (wkandi01), 2/21/20     
*     hw4                                                                    
*                                                                              
*     description: The implementation of compress40.h in which the functions 
*                  are implemented
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "seq.h"
#include "assert.h"
#include "compress40.h"
#include "bitpack.h"
#include "pnm.h"
#include "PNMandCV.h"
#include "a2methods.h"
#include "a2plain.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "CVandCosine.h"
#include "CosineandWord.h"

Pnm_ppm readFile(FILE *fp, A2Methods_T methods);
int checkWidth(Pnm_ppm uarray2);
int checkHeight(Pnm_ppm uarray2);

/*  Function - compress40 
 *  Arguments: file pointer representing the image being read in
 *  Purpose: Asserts file pointer is not null, reads in picture, calls various
 *  compreesion modules
 */
extern void compress40(FILE *input) 
{
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);
    if (input == NULL) {
        fprintf(stderr, "%s\n", "Image file could not be opened");
        exit(EXIT_FAILURE);
    }
    Pnm_ppm RGBarray = Pnm_ppmread(input, methods);
    assert(RGBarray);
    int width = checkWidth(RGBarray);
    int height = checkHeight(RGBarray);
    UArray2_T CVUArray = PnmToUArray(RGBarray, width, height, methods);
    Seq_T CosineSeq = CVUArrayToCosineSeq(CVUArray, width, height);
    printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
    CosineSeqtoWord(CosineSeq);
    UArray2_free(&CVUArray);
    Seq_free(&CosineSeq);
    Pnm_ppmfree(&RGBarray);
} 

/*  Function - decompress40 
 *  Arguments: file pointer representing the compressed image file
 *  Purpose: Asserts file pointer is not null, reads in picture, calls various
 *  decompression modules
 */
extern void decompress40(FILE *input) 
{
    assert(input != NULL);
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods);
    unsigned height, width;
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u\n", 
                    &width, &height);
    assert(read == 2);
    UArray2_T WordArray = read_word(input, width, height);
    Seq_T returnCosineSeq = WordstoCosineSeq(WordArray, width, height);
    UArray2_T returnCV = CosineSeqtoCVUArray2(returnCosineSeq, width, height);
    Pnm_ppm returnRGB = UArraytoPnm(returnCV, width, height, methods);
    Pnm_ppmwrite(stdout, returnRGB);
    UArray2_free(&WordArray);
    Seq_free(&returnCosineSeq);
    UArray2_free(&returnCV);
    Pnm_ppmfree(&returnRGB);
}

/*  Function - checkwidth 
 *  Arguments: a Ppm_pnm object representing the image
 *  Return: the new width of the image if changed
 *  Purpose: to make sure width is an even number
 */
int checkWidth(Pnm_ppm uarray2)
{
    int width = uarray2->width;
    if (width % 2 != 0) {
        width--;
    }
    return width;
}

/*  Function - checkheight 
 *  Arguments: a Ppm_pnm object representing the image
 *  Return: the new width of the image if changed
 *  Purpose: to make sure height is an even number
 */
int checkHeight(Pnm_ppm uarray2)
{
    int height = uarray2->height;
    if (height % 2 != 0) {
        height--;
    }
    return height;
}