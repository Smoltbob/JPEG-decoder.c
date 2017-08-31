#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include "bitstream.h"
#include "jpeg_reader.h"

/* choix de la structure*/
struct huff_table {
    /* true si c'est une feuille*/
    bool feuille;
    uint8_t code;
    /* pointeur vers le pere*/
    struct huff_table *pere;
    /* fils gauche à 0, fils droit à 1*/
    struct huff_table *fils[2];
};

struct huff_table *init_branches(struct huff_table *racine)
{
    struct huff_table *fils=malloc(sizeof(struct huff_table));
    fils->feuille = false;
    fils->pere = racine;
    fils->fils[0] = NULL;
    fils->fils[1] = NULL;
    return fils;
}

struct huff_table *load_huffman_table(struct bitstream *stream, uint16_t *nb_byte_read)
{
    /* parcourir le flux de bits stream pour construire une table de Huffman*/
    struct huff_table *huffman_table=malloc(sizeof(struct huff_table));
    huffman_table->feuille = false;
    huffman_table->pere = NULL;
    huffman_table->fils[0] = NULL;
    huffman_table->fils[1] = NULL;
    /* /!\ tableau contenant le nombre n[i+1] (nombres de codes de longeur 0<=i<=15)*/
    uint32_t dest[16];
    uint16_t somme = 0;
    for (uint8_t i = 0; i < 16; i++){
        *nb_byte_read += read_bitstream(stream, 8, &dest[i], false);
        somme += dest[i];
    }
    /* tableau contenant les symboles */
    uint32_t symbole[somme];
    for (uint8_t j = 0; j < somme; j++) {
        /* un symbole est codé sur 1 octet */
        *nb_byte_read += read_bitstream(stream, 8, &symbole[j], false);
    }
    uint8_t k = 0;
    uint8_t descendu=0;
    for (uint8_t i = 0; i < 16; i++) {
        uint32_t nombre_symboles = dest[i];
        uint8_t a_descendre = i + 1 - descendu;
        for (uint8_t j = 0; j < nombre_symboles; j++) {
            while (a_descendre > 0) {
                if (huffman_table->fils[0] == NULL) {
                    struct huff_table *huffman_tablefg = init_branches(huffman_table);
                    huffman_table->fils[0] = huffman_tablefg;
                    huffman_table = huffman_table->fils[0];
                    a_descendre = a_descendre - 1;
                    descendu +=1;
                } else if(huffman_table->fils[1] == NULL) {
                    struct huff_table *huffman_tablefd = init_branches(huffman_table);
                    huffman_table->fils[1] = huffman_tablefd;
                    huffman_table = huffman_table->fils[1];
                    a_descendre = a_descendre - 1;
                    descendu +=1;
                } else {
                    huffman_table = huffman_table->pere;
                    a_descendre = a_descendre + 1;
                    descendu -=1;
                }
            }
            huffman_table->feuille = true;
            huffman_table->code = symbole[k];
            k = k + 1;
            huffman_table = huffman_table->pere;
            a_descendre = a_descendre + 1;
            descendu -=1;
        }
    }
    while (huffman_table->pere != NULL) {
        huffman_table = huffman_table->pere;
    }
    return huffman_table;
}

int8_t next_huffman_value(struct huff_table *table,struct bitstream *stream)
{
    /* TODO retourne la prochaine valeur atteinte en parcourant la table de huffman
       "table" selon les bits extraits du flux stream */
    while (! table->feuille) {
        uint32_t bit_cour;
        read_bitstream(stream, 1, &bit_cour, true);
        if (table->fils[bit_cour] == NULL){
          fprintf(stderr, "Invalid Huffman sequence in bit stream \n");
          exit(1);
        }
        table = table->fils[bit_cour];
    }
    int8_t code = (int8_t)table->code;
    while (table->pere != NULL) {
        table = table->pere;
    }
    return code;
}

void free_huffman_table(struct huff_table *table)
{
    if (table != NULL) {
        if (table->feuille != true) {
            free_huffman_table(table->fils[0]);
            free_huffman_table(table->fils[1]);
        }
        free(table);
    }
}
