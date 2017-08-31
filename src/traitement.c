#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <extracteur.h>
#include "jpeg_reader.h"
#include "clampe.h"
#include "structure.h"
#include "traitement.h"
#define PI 3.14159265

void quantification_inverse(int16_t *tab_frequences, uint8_t *qtable)
{
    /* table de quantification Y*/

    /* table de quantification Cb et Cr*/
    /* tableau qui pointe vers les tables de quantification */
    for (int8_t i = 0; i < 64; i++) {
        /* quantification inverse de Y*/
        tab_frequences[i] = tab_frequences[i]*qtable[i];
    }
}


uint8_t idx_zigzag(uint8_t idx)
{
    /*
       calcule les nouveaux indices du zigzag
       le tableau est de taille 64, c'est un bloc
       */
    static uint8_t tab[] = {0,   1,  8, 16,  9,  2, 3, 10,
                            17, 24, 32, 25, 18, 11,  4, 5,
                            12, 19, 26, 33, 40, 48, 41, 34,
                            27, 20, 13,  6,  7, 14, 21, 28,
                            35, 42, 49, 56, 57, 50, 43, 36,
                            29, 22, 15, 23, 30, 37, 44, 51,
                            58, 59, 52, 45, 38, 31, 39, 46,
                            53, 60, 61, 54, 47, 55, 62, 63
                           };
    return tab[idx];
}


void zigzag_inverse(int16_t *tab_frequences)
{
    uint16_t idx = 0;
    int16_t tmp[64] = {0};
    for (uint8_t i = 0; i < 64; i++) {
        tmp[i] = tab_frequences[i];
    }
    for (uint16_t i=0; i<64; i++) {
        idx = idx_zigzag(i);
        tab_frequences[idx] = tmp[i];
    }
}

/* Cette fonction calcule les coefficients de la fonction iDCT pour optimiser le programme */
float **tableau_cosinus()
{
    float **tab_matrice_cos = malloc(64*sizeof(float *));
    for (int8_t i = 0; i < 64; i++) {
        float *tmp = malloc(64*sizeof(float));
        tab_matrice_cos[i] = tmp;
    }
    for (int8_t y = 0; y < 8; y++) {
        for (int8_t x = 0; x < 8; x++) {
            for (int8_t j = 0; j < 8; j++) {
                for (int8_t i = 0; i < 8; i++) {
                    (tab_matrice_cos[8*y+x])[8*j+i] = cos((2*x+1)*i*PI/16)*cos((2*y+1)*j*PI/16);
                }
            }
        }
    }
    return tab_matrice_cos;
}

float coeff(uint8_t i)
{
    if (i == 0) {
        return 1/sqrt(2);
    } else {
        return 1;
    }
}

/* Cette fonction calcule le bloc spatial, on rajoutant l'offset de 128
   et en mettant S(x,y) = 0 si S(x,y) < 0 ou S(x,y) = 255 si S(x,y) > 255 */
void iDCT(int16_t *tab_frequences)
{
    float **tab_cosinus = tableau_cosinus();
    int16_t aux[64];
    for (uint8_t i=0; i<64; i++) {
        aux[i] = tab_frequences[i];
    }
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            float tmp = 0;
            for (uint8_t j = 0; j < 8; j++) {
                for (uint8_t i = 0; i < 8; i++) {
                    tmp += coeff(j)*coeff(i)*((tab_cosinus[8*y+x])[8*j+i]*aux[8*j + i]);
                }
            }
            tmp = tmp/4 + 128;
            tab_frequences[8*y + x] = clampe(tmp);
        }
    }
    for (int8_t i = 0; i < 64; i++) {
        free(tab_cosinus[i]);
    }
    free(tab_cosinus);
}


/* ALGORITHME DE LOEFFLER*/

void butterfly_unit(float *tab8_freq, int8_t y0, int8_t y1)
{
    float tmp1 = (tab8_freq[y0] + tab8_freq[y1])/2;
    float tmp2 = (tab8_freq[y0] - tab8_freq[y1])/2;
    tab8_freq[y0] = tmp1;
    tab8_freq[y1] = tmp2;
}

void rotator_unit(float *tab8_freq, int8_t y0, int8_t y1, int8_t n, float k)
{
    float tmp1 = tab8_freq[y0]/k;
    float tmp2 = tab8_freq[y1]/k;
    tab8_freq[y0] = tmp1*cos(n*PI/16) - tmp2*sin(n*PI/16);
    tab8_freq[y1] = tmp2*cos(n*PI/16) + tmp1*sin(n*PI/16);
}

static uint8_t tab1[] = {0,4,2,6,7,3,5,1};

uint8_t reorganiser(uint8_t i)
{
    return tab1[i];
}

float *iDCT_1D(float *tab8_freq)
{
    for (uint8_t i = 0; i < 8; i++) {
        tab8_freq[i] = tab8_freq[i]*sqrt(8);
    }
    /* Si (stage == 1) */
    tab8_freq[3] = tab8_freq[3]/sqrt(2);
    tab8_freq[5] = tab8_freq[5]/sqrt(2);
    butterfly_unit(tab8_freq, 1, 7);

    /* Si (stage == 2) */
    butterfly_unit(tab8_freq, 0, 4);
    rotator_unit(tab8_freq, 2, 6, 6, sqrt(2));
    butterfly_unit(tab8_freq, 7, 5);
    butterfly_unit(tab8_freq, 1, 3);

    /* Si (stage == 3) */
    butterfly_unit(tab8_freq, 0, 6);
    butterfly_unit(tab8_freq, 4, 2);
    rotator_unit(tab8_freq, 7, 1, 3, 1);
    rotator_unit(tab8_freq, 3, 5, 1, 1);

    /* Si (stage == 4) */
    butterfly_unit(tab8_freq, 6, 7);
    butterfly_unit(tab8_freq, 2, 3);
    butterfly_unit(tab8_freq, 4, 5);
    butterfly_unit(tab8_freq, 0, 1);

    float *tab_iDCT = malloc(8*sizeof(float));
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t x = reorganiser(i);
        tab_iDCT[i] = tab8_freq[x];
    }
    return tab_iDCT;
}


void iDCT_loeffler(int16_t *tab_frequences){
    /* iDCT_loeffler en lignes */
    float tableau_frequences[8][8];
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            tableau_frequences[y][x] = tab_frequences[8*y+x];
        }
        float *tab_tmp = iDCT_1D(tableau_frequences[y]);
        for (uint8_t i = 0; i < 8; i++) {
            tableau_frequences[y][i] = tab_tmp[i];
        }
        free(tab_tmp);
    }

    /* iDCT_loeffler en colonnes */
    float tableau2_frequences[8][8];
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            tableau2_frequences[y][x] = tableau_frequences[x][y];
        }
        float *tab_tmp = iDCT_1D(tableau2_frequences[y]);
        for (uint8_t i = 0; i < 8; i++) {
            tableau2_frequences[y][i] = tab_tmp[i];
        }
        free(tab_tmp);
    }

    /* on réorganise les lignes en colonnes*/
    float tableau3_frequences[64];
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 8; x++) {
            tableau3_frequences[8*y+x] = tableau2_frequences[x][y];
        }
    }

    for (uint8_t y = 0; y < 64; y++) {
        tab_frequences[y] = clampe(tableau3_frequences[y] +128);
    }
}


void traiter_mcu(struct mcu* mcu, struct jpeg_desc *jdesc)
{
    uint8_t *tcompos = mcu->t_compos;
    int16_t **ptcompos[3] = {mcu->composantes.Y, mcu->composantes.Cb, mcu->composantes.Cr};
    /* Pour chaque composante .. (Y Cb Cr) */
    for (uint8_t i = 0; i < mcu->nbcomp; i++) {
        /* Pour chaque bloc de composante .. (ex 2Y, 1Cb, 1Cr) */
        for (uint8_t j = 0; j < tcompos[i]; j++) {
            /* quantification inverse */
            uint8_t *qtable = get_quantization_table(jdesc, i != 0);
            quantification_inverse(ptcompos[i][j], qtable);
            /* zigzag inverse pour reordonnancement */
            zigzag_inverse(ptcompos[i][j]);
            /* idct */
            iDCT_loeffler(ptcompos[i][j]);

        }
    }
}
