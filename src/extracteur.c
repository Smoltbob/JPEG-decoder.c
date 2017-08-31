/* Module d'extraction des blocs */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "huffman.h"
#include "jpeg_reader.h"
#include "structure.h"
#include "bitstream.h"

/* Décode une valeur encodée en magnitude */
int16_t dec_magnitude(uint8_t magnitude, uint16_t val)
{
    /* Pour une magnitude m, les valeurs sont comprises dans
     * [-2^m + 1; -2^(m-1)] ou [2^(m-1); 2^m - 1].
     * Pour savoir si on est négatif ou positif on regarde le bit de poids
     * fort de la valeur encodée.
     * Puis on traite la valeur encodée en utilisant le reste des bits
     */
    uint8_t size = magnitude;
    int16_t res = 0;
    uint16_t pdsfort = (1 << (size - 1)) & val;
    pdsfort = pdsfort >> (size - 1);
    if (magnitude == 1)
        res = val == 0 ? -1 : 1;
    else if (magnitude != 0) {
        if (pdsfort == 0)
            res = -(2 << (magnitude -1)) + 1 + ((0xFFFF >> (16 - size + 1)) & val);
        else
            res = (2 << (magnitude - 2)) + ((0xFFFF >> (16 - size + 1)) & val);
    }
    return res;
}

/* Décode un symbole encodé en RLE.
 * La magnitude décodée et la taille du vecteur retourné seront stockées dans
 * les variables de même nom. totalecr sert à savoir combien de zéros doit-on
 * écrire en cas de End of Block (il faut remplir le reste du bloc de 0 */
int16_t *dec_RLE(uint16_t symbole, uint8_t *magnitude, uint8_t *size, uint8_t totalecr)
{
    int16_t *res = NULL;
    /* End of Block : 0x00*/
    if (symbole == 0) {
        res = calloc(63 - totalecr, sizeof(int16_t));
        *size = 63 - totalecr;
    }
    /* ZRL */
    else if (symbole == 0xF0) {
        res = calloc(16, sizeof(int16_t));
        *size = 16;
    }
    /* 0x?0 : code invalide */
    else if (symbole << 12 == 0) {
        printf("Code RLE invalide");
        *size = 0;
        exit(EXIT_FAILURE);
    }
    /* 0xAB : A composantes nulles, B magnitude */
    else {
        uint8_t compnulles = (0xF0 & symbole) >> 4;
        if (compnulles >= 63 - totalecr)
            compnulles = 63 - totalecr -1;
        *size = compnulles;
        *magnitude = 0x0F & symbole;
        res = calloc(compnulles, sizeof(int16_t));
    }
    return res;
}

/* Lit le bitstream avec gestion du byte stuffing par défaut et gestion
 * des erreurs de lecture */
uint8_t sread_bitstream(struct bitstream *stream, uint8_t val, uint32_t* byte)
{
    uint8_t nb_read = read_bitstream(stream, val, byte, true);
    if (nb_read != val) {
        fprintf(stderr, "[bitsream.c] Read error ");
        fprintf(stderr, "(tried reading %hhu bits, only got %u)\n", val, nb_read);
        exit(EXIT_FAILURE);
    }
    return nb_read;
}

/* Décode 63 coefficients AC */
int16_t *lire_AC(struct huff_table *htable, struct bitstream *stream)
{
    uint8_t octetslus = 0;
    int16_t val = 0;
    uint32_t byte = 0;
    uint8_t magnitude = 0;
    uint8_t size = 0;
    int16_t *resACtab = 0;
    int16_t *res = calloc(64, sizeof(int16_t));
    int8_t j = 1;
    while(octetslus != 63) {
        val = next_huffman_value(htable, stream);
        resACtab = dec_RLE(val, &magnitude, &size, octetslus);
        octetslus += size;
        for (int i = 0; i < size; i++) {
            res[j] = resACtab[i];
            j++;
        }
        if (val != 0 && val != 0xF0) {
            sread_bitstream(stream, magnitude, &byte);
            res[j] = dec_magnitude(magnitude, byte);
            octetslus++;
            j++;
        }
        free(resACtab);
    }
    return res;
}

/* Décode un coefficient DC */
int16_t lireDC(struct huff_table *htable, struct bitstream *stream)
{
    uint32_t byte = 0;
    int16_t val = next_huffman_value(htable, stream);
    sread_bitstream(stream, val, &byte);
    return dec_magnitude(val, byte);
}

/* Lit et décode un vecteur de 64 valeurs pour une certaine composante */
int16_t *lire_bloc(struct jpeg_desc *jdesc, struct bitstream *stream, enum component comp)
{
    /* Variable statique pour ajouter à DC la valeur continue du bloc préc */
    static int16_t DCprec[3] = {0};
    /* Récupérer les tables de Huffman associées au bloc */
    struct huff_table *htableAC = NULL;
    struct huff_table *htableDC = NULL;
    uint8_t index = get_frame_component_quant_index(jdesc, comp);
    htableDC = get_huffman_table(jdesc, DC, index);
    htableAC = get_huffman_table(jdesc, AC, index);
    /* Lecture de la magnitude Y DC */
    int16_t resDC = lireDC(htableDC, stream);
    /* Lecture des 63 coefficients AC */
    int16_t *res = lire_AC(htableAC, stream);

    /* Ajout de DC dans le bloc */
    res[0] = resDC + DCprec[comp];
    DCprec[comp] = res[0];
    return res;
}

/* Retourne quel sera l'ordre des blocs dans le fichier.
 * Nous voulons pouvoir stocker les composantes Y, Cb, Cr dans un tableau.
 * Indice 0 : Y, 1 : Cb, 2 : Cr systématiquement quel que soit l'ordre
 * d'apparition des composantes.
 * Cette fonction indique dans quel ordre il faudra utiliser les indices pour
 * respecter la convention précédente.
 * Ex: si l'ordre des composantes est Cb Y Cr on  commencera par écrire le
 * premier bloc à l'indice 1, puis 0, puis 2.
 * */
void *ordre_compo(struct jpeg_desc *jdesc, uint8_t* ordre)
{
    /* Il faut connaître l'ordre des blocs */
    /* Identifiants de chaque composante */
    uint8_t idY = get_frame_component_id(jdesc, COMP_Y);
    uint8_t idCb = get_frame_component_id(jdesc, COMP_Cb);

    /* Pour chaque position on veut connaître l'id correspondant */
    uint8_t idprem = get_scan_component_id(jdesc, 0);
    uint8_t iddeux = get_scan_component_id(jdesc, 1);

    if (idprem == idY) {
        //Première composante Y
        ordre[0] = 0;
        //Deuxième composante Cb
        ordre[1] = iddeux == idCb ? 1:2;
        //Troisième composante Cr
        ordre[2] = iddeux == idCb ? 2:1;
    } else if (idprem == idCb) {
        //Première composante Cb
        ordre[0] = 1;
        //Deuxième composante Y
        ordre[1] = iddeux == idY ? 0:2;
        //Troisième composante Cr
        ordre[2] = iddeux == idY ? 2:0;
    } else {
        //Première composante Cr
        ordre[0] = 2;
        //Deuxième composante Cb
        ordre[1] = iddeux == idCb ? 1:0;
        //Troisième composante Y
        ordre[2] = iddeux == idCb ? 0:1;
    }
    return ordre;
}

/* Retourne combien de blocs on aura par composante. Ex : YYCbCr -> 2, 1, 1
 * Nous allons lire le nombre de composantes et initialiser un tableau de
 * cette taille (ce sera donc 1 ou 3).
 * A chaque case du tableau correspond une le nombre de blocs qui forme une
 * composante. Si il n'y a pas de sous-échantillonnage il n'y aura que des 1.
 */
uint8_t *nb_blocs(struct jpeg_desc *jdesc)
{
    uint8_t nbcomp = get_nb_components(jdesc);
    uint8_t *res = malloc(nbcomp*sizeof(int8_t));
    /* Lisons le nombre de blocs pour chaque composante.
     * Indice de lecture pour Y : 0. Pour Cb / Cr : 1 */
    for (uint8_t j = 0; j < nbcomp; j++) {
        uint8_t sampV = get_frame_component_sampling_factor(jdesc, DIR_V, j!=0);
        uint8_t sampH = get_frame_component_sampling_factor(jdesc, DIR_H, j!=0);
        uint8_t nbblocs = sampV * sampH;
        res[j] = nbblocs;
    }
    return res;
}

/* Décode une MCU entière.
 * Pour une image noir et blanc on décode un bloc Y.
 * Pour une image couleur non sous-échantilonnée on décode trois blocs YCbCr.
 * Pour une image sous échantillonnée on retourne YYYYCbCr par exemple.
 */
struct mcu decoder(struct jpeg_desc *jdesc, struct bitstream *stream)
{
    if (end_of_bitstream(stream) == true) {
        printf("Error : no more data to read\n");
        exit(EXIT_FAILURE);
    }

    uint8_t nbcomp = get_nb_components(jdesc);
    uint8_t *nbblocs = nb_blocs(jdesc);
    int16_t ***res = calloc(3, sizeof(int16_t **));

    /* Faisons correspondre les id des positions aux id des composantes pour
     * connaître l'ordre de lecture */
    uint8_t *ordre = malloc(nbcomp * sizeof(uint8_t));
    if (nbcomp == 3)
        ordre_compo(jdesc, ordre);
    else
        ordre[0] = 0;
    /* On itère sur les composantes puis les blocs par composantes */
    for (uint8_t i = 0; i < nbcomp; i++) {
        res[i] = malloc(nbblocs[i] * sizeof(int16_t *));
        for (uint8_t j = 0; j < nbblocs[i]; j++) {
            res[ordre[i]][j] = lire_bloc(jdesc, stream, i);
        }
    }
    struct tabcomposantes compo = {res[0], res[1], res[2]};
    struct mcu mcu = {compo, nbblocs, nbcomp};
    free(res);
    free(ordre);
    return mcu;
}

/* Libère les éléments d'une mcu et la mcu elle-même */
uint8_t free_mcu(struct mcu mcu)
{
    int16_t **ptcompos[3] = {mcu.composantes.Y, mcu.composantes.Cb, mcu.composantes.Cr};
    for (uint8_t i = 0; i < mcu.nbcomp; i++) {
        for (uint8_t j = 0; j < mcu.t_compos[i]; j++) {
            free(ptcompos[i][j]);
        }
        free(ptcompos[i]);
    }
    free(mcu.t_compos);
    return 0;
}
