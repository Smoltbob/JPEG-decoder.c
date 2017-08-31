#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "huffman.h"

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

static struct huff_table *init_branches(struct huff_table *racine)
{
    struct huff_table *fils=malloc(sizeof(struct huff_table));
    fils->feuille = false;
    fils->pere = racine;
    fils->fils[0] = NULL;
    fils->fils[1] = NULL;
    return fils;
}


struct huff_table *load_huffman_table1(uint32_t *dest, uint32_t *symbole)
{
    /* parcourir le flux de bits stream pour construire une table de Huffman*/
    struct huff_table *huffman_table=malloc(sizeof(struct huff_table));
    huffman_table->feuille = false;
    huffman_table->pere = NULL;
    huffman_table->fils[0] = NULL;
    huffman_table->fils[1] = NULL;
    /* /!\ tableau contenant le nombre n[i+1] (nombres de codes de longeur 0<=i<=15)*/
    uint16_t somme = 0;
    for (uint8_t i = 0; i < 16; i++) {
        somme += dest[i];
    }
    /* tableau contenant les symboles */
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

int main()
{
    uint32_t dest[16] = {0,2,2,4,0,0,0,0,0,0,0,0,0,0,0,0};
    for (uint8_t i = 0; i < 16; i++) {
        printf(" Il y a %x symboles de longeur : %d\n", dest[i], i);
    }
    uint32_t symbole[8] = {1,2,3,4,5,6,7,8};
    printf(" Les symboles sont :");
    for (uint8_t i = 0; i < 8; i++) {
        printf(" %x ", symbole[i]);
    }
    struct huff_table *huffman = load_huffman_table1(dest, symbole);
    printf("\n Apres avoir créé la table de huffman, on affiche les symboles :\n");
    printf("huffman->fils[0]->fils[0]->code : %d\n", huffman->fils[0]->fils[0]->code);
    printf("huffman->fils[0]->fils[1]->code  :%d\n", huffman->fils[0]->fils[1]->code);
    printf("huffman->fils[1]->fils[0]->fils[0]->code : %d\n", huffman->fils[1]->fils[0]->fils[0]->code);
    printf("huffman->fils[1]->fils[0]->fils[1]->code : %d\n", huffman->fils[1]->fils[0]->fils[1]->code);
    printf("huffman->fils[1]->fils[1]->fils[0]->fils[0]->code : %d\n", huffman->fils[1]->fils[1]->fils[0]->fils[0]->code);
    printf("huffman->fils[1]->fils[1]->fils[0]->fils[1]->code : %d\n", huffman->fils[1]->fils[1]->fils[0]->fils[1]->code);
    printf("huffman->fils[1]->fils[1]->fils[1]->fils[0]->code : %d\n", huffman->fils[1]->fils[1]->fils[1]->fils[0]->code);
    printf("huffman->fils[1]->fils[1]->fils[1]->fils[1]->code : %d\n", huffman->fils[1]->fils[1]->fils[1]->fils[1]->code);
    printf("\n");
    free_huffman_table(huffman);
}
