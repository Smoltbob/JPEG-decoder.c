#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "bitstream.h"

/* Initialise un bitstream et lit nbbits, pour faire des tests */
uint32_t lire(uint8_t nbbits, char *filename)
{
    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    /* Create Bitstream */
    struct bitstream *stream = create_bitstream(filename);
    uint32_t bitslire = 0;
    uint8_t lus = read_bitstream(stream, nbbits, &bitslire, false);
    printf("Nombre de bits venant d'être lus : %hhu\n", lus);
    close_bitstream(stream);
    return bitslire;
}

/* Lit successivement de 0 */
void test_readbits(char *filename)
{
    uint32_t val = 0;
    for (uint8_t i = 0; i <= 32; i++) {
        val = 0;
        printf("Lecture de %hhu bits.\n",i);
        val = lire(i, filename);
        printf("Valeur : %x \n", val);
    }
    printf("Test OK\n");
}

/* Lit un fichier en entier en utilisant*/
void test_lire_fichier(char *filename)
{
    uint32_t val = 0;
    struct bitstream *streamp = create_bitstream(filename);
    uint32_t id = 1;
    while (end_of_bitstream(streamp) == false) {
        uint8_t nbread = read_bitstream(streamp, 8, &val, true);
        if(end_of_bitstream(streamp) == false) {
            printf("Lu : %x, ", val);
            printf("nbread : %hhu\n", nbread);
        }
        if(id % 8 == 0 && id > 0)
            printf("\n");
        id++;
    }
    printf("\n");
    close_bitstream(streamp);
}

/* Teste le skip_until */
void test_skip(char *filename, uint8_t byte)
{
    uint32_t val2 = 0;
    struct bitstream *stream = create_bitstream(filename);
    skip_bitstream_until(stream, byte);
    uint8_t nbread = read_bitstream(stream, 8, &val2, false);
    printf("Prof\nValeur : %x, Bits lus : %hhu\n", val2, nbread);
    close_bitstream(stream);
}

/* Prototypage pour bitstream */
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* On recupere le nom du fichier JPEG sur la ligne de commande. */
    /* Create Bitstream */
    char *filename = argv[1];

    struct bitstream* stream = create_bitstream(filename);
    close_bitstream(stream);
    test_readbits(filename);
    test_lire_fichier(filename);
    test_skip(filename, 0xFF);
}
