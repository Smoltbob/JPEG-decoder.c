#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bitstream.h"
#include "huffman.h"
#include "jpeg_reader.h"

/* Quelques types enumeres et structure */
struct jpeg_desc {
  /* DQT */
  /* a malloc */
  uint8_t nb_quantization_tables;
  struct table *qtables;
  /* SOF */
  uint16_t hauteur;
  uint16_t largeur;
  uint8_t nb_comp;
  /* id composantes */
  uint8_t iCY;
  uint8_t iCCb;
  uint8_t iCCr;
  /* sampling factor horizontaux */
  uint8_t SFHY;
  uint8_t SFHCb;
  uint8_t SFHCr;
  /* verticaux */
  uint8_t SFVY;
  uint8_t SFVCb;
  uint8_t SFVCr;
  /* indice de table de quantification */
  uint8_t iQY;
  uint8_t iQCb;
  uint8_t iQCr;
  /* DHT */
  uint8_t nb_huff_ac;
  uint8_t nb_huff_dc;
  struct huff_table **huff_table_ac;
  struct huff_table **huff_table_dc;
  /* SOS */
  /* scan id */
  uint8_t iY;
  uint8_t iCb;
  uint8_t iCr;
  /* indice des tables de Huffman */
  uint8_t iDCY;
  uint8_t iACY;
  uint8_t iDCCb;
  uint8_t iACCb;
  uint8_t iDCCr;
  uint8_t iACCr;
  /* inutile uint8_t precision; */
  char *filename;
  struct bitstream* stream;
  bool full;
};

struct table {
  /* tableau de uint8_t */
  uint8_t *ptr;
};

/* le flux pointe après le marqeur */
uint32_t skip_length(struct bitstream *stream) {
  uint32_t c;
  read_bitstream(stream, 16, &c, true);
  return c-2;
}

int read_marker(struct bitstream* stream) {
  uint32_t c;
  read_bitstream(stream, 16, &c, true);
  return (int)c;
}

void read_app0(struct bitstream* stream) {
  // printf("***APP0***\n");
  /* on skip la taille */
  skip_length(stream);
  /*
  verification de la chaine "JFIF\0"
  caractere par caractere
  */
  char chaine[10] = "";
  uint32_t c;
  for (uint8_t i=0; i<4; i++) {
    read_bitstream(stream, 8, &c, true);
    strncat(chaine, (char*)&c, 1);
  }
  if (strncmp(chaine, "JFIF", 5) != 0) {
    printf("Error : format JFIF not detected in section APP0\n");
    exit(1);
  }
  /* on saute a la section suivante */
  skip_bitstream_until(stream, 0xff);
}

/*
skip la section COM si elle existe
le flux est sur le marqueur de la section
*/
void skip_comment(struct bitstream* stream) {
  // printf("***COM***\n");
  skip_bitstream_until(stream, 0xff);
}

/* section DQT */
void read_dqt(struct bitstream* stream, struct jpeg_desc* jdesc) {
  // printf("***DQT***\n");
  /* longueur */
  uint32_t length =   skip_length(stream);
  /* precision */
  uint32_t precision, iQ;
  for (uint32_t i=0; i<length; i+=65) {
    read_bitstream(stream, 4, &precision, true);
    if (precision != 0) {
      printf("Wrong precision for quantification table, expected 1 byte but found two\n");
      exit(1);
    }
    /* table de quantification */
    struct table qtable;
    qtable.ptr = malloc(64*sizeof(uint8_t));
    read_bitstream(stream, 4, &iQ, true);
    for (uint8_t i=0; i<64; i++) {
      uint32_t tmp;
      read_bitstream(stream, 8, &tmp, true);
      qtable.ptr[i] = tmp;
    }
    jdesc->qtables[iQ] = qtable;
    jdesc->nb_quantization_tables ++;
  }
  // skip_bitstream_until(stream, 0xff);
}

#define sof_comp(jdesc, iC, SFH, SFV, iQ)               \
do {                                        \
  uint32_t tmp;                             \
  read_bitstream(stream, 8, &tmp, true);    \
  jdesc->iC = (uint8_t)tmp;             \
  read_bitstream(stream, 4, &tmp, true);    \
  jdesc->SFH = (uint8_t)tmp;            \
  read_bitstream(stream, 4, &tmp, true);    \
  jdesc->SFV = (uint8_t)tmp;            \
  read_bitstream(stream, 8, &tmp, true);    \
  jdesc->iQ = (uint8_t)tmp;             \
} while (0);

void read_sof(struct bitstream* stream, struct jpeg_desc *jdesc) {
  // printf("***SOF***\n");
  skip_length(stream);
  uint32_t tmp;
  read_bitstream(stream, 8, &tmp, true);
  if (tmp != 8) {
    printf("Wrong precision, expected 1 byte but found two\n");
    exit(1);
  }
  read_bitstream(stream, 16, &tmp, true);
  jdesc->hauteur = (uint16_t)tmp;
  read_bitstream(stream, 16, &tmp, true);
  jdesc->largeur = (uint16_t)tmp;
  read_bitstream(stream, 8, &tmp, true);
  jdesc->nb_comp = (uint8_t)tmp;
  sof_comp(jdesc, iCY, SFHY, SFVY, iQY);
  if (jdesc->nb_comp == 3) {
    sof_comp(jdesc, iCCb, SFHCb, SFVCb, iQCb);
    sof_comp(jdesc, iCCr, SFHCr, SFVCr, iQCr);
  }
}

void read_dht(struct bitstream* stream, struct jpeg_desc* jdesc) {
  // printf("***DHT***\n");
  uint32_t length = skip_length(stream);
  uint32_t tmp;
  /* tableau donnant le nombre de codes pour une longueur donnee */
  struct huff_table* cour;
  uint16_t tmp2=0;
  uint16_t* nb_byte_read = &tmp2;
  uint16_t nb_read=0;
  while (nb_read < length) {
    /* au prealable */
    read_bitstream(stream, 3, &tmp, true);
    if (tmp != 0) {
      printf("Error in DHT section\n");
      exit(1);
    }
    uint32_t iACDC;
    read_bitstream(stream, 1, &iACDC, true);
    uint32_t iH;
    read_bitstream(stream, 4, &iH, true);
    /* construction de la table */
    cour = load_huffman_table(stream, nb_byte_read);
    /* lus nb_byte_read + 1 octet d'info sur la table */
    nb_read += *nb_byte_read+1;
    /* table DC */
    if (iACDC == 0) {
      jdesc->huff_table_dc[iH] = cour;
      jdesc->nb_huff_dc++;
    } else {
      /* table AC */
      jdesc->huff_table_ac[iH] = cour;
      jdesc->nb_huff_ac++;
    }
  }
}

#define sos_comp(jdesc, idx, iDC, iAC)           \
do {                                             \
  uint32_t tmp;                                  \
  jdesc->idx = i;                                \
  read_bitstream(stream, 4, &tmp, true);    \
  jdesc->iDC = (uint8_t)tmp;                \
  read_bitstream(stream, 4, &tmp, true);    \
  jdesc->iAC = (uint8_t)tmp;                \
} while (0);

void read_sos(struct bitstream* stream, struct jpeg_desc* jdesc) {
  // printf("***SOS***\n");
  skip_length(stream);
  uint32_t tmp;
  read_bitstream(stream, 8, &tmp, true);
  if ((uint8_t)tmp != jdesc->nb_comp) {
    printf("Error : different number of components between SOS and SOF\n");
    exit(1);
  }
  uint32_t iC;
  for (uint8_t i=0; i<jdesc->nb_comp; i++) {
    read_bitstream(stream, 8, &iC, true);
    if ((uint8_t) iC == jdesc->iCY) {
      sos_comp(jdesc, iY, iDCY, iACY);
    } else if (jdesc->nb_comp == 3 && iC == jdesc->iCCb) {
      sos_comp(jdesc, iCb, iDCCb, iACCb);
    } else if (jdesc->nb_comp == 3 && iC == jdesc->iCCr) {
      sos_comp(jdesc, iCr, iDCCr, iACCr);
    } else {
      printf("Error : wrong scan id\n");
      exit(1);
    }
  }
  /* amene le flux au debut des donnees brutes */
  read_bitstream(stream, 24, &tmp, true);
  jdesc->full = true;
}

/* creation et initialisation du descripteur */
struct  jpeg_desc*  create_desc(const char *filename, struct bitstream* stream) {
  struct  jpeg_desc *jdesc = malloc(sizeof(struct  jpeg_desc));
  jdesc->full = false;
  /* allocation des tables */
  struct table *qtables = calloc(4, sizeof(struct table));
  jdesc->qtables = qtables;
  jdesc->nb_quantization_tables = 0;
  struct huff_table** huff_table_dc = calloc(4, sizeof(struct huff_table*));
  struct huff_table** huff_table_ac = calloc(4, sizeof(struct huff_table*));
  jdesc->huff_table_dc = huff_table_dc;
  jdesc->huff_table_ac = huff_table_ac;
  jdesc->nb_huff_dc=0;
  jdesc->nb_huff_ac=0;
  jdesc->filename = (char*)filename;
  jdesc->stream = stream;
  return jdesc;
}

/*
cree un descripteur et le remplit en lisant toutes les entetes JPEG

Nota:
la lecture est stoppee a la fin de l'entete SOS (=debut donnees brutes)
*/
struct  jpeg_desc * read_jpeg(const char *filename) {
  /* creation du bitstream et initialisation du descripteur */
  struct bitstream *stream = create_bitstream(filename);
  struct  jpeg_desc* jdesc = create_desc(filename, stream);
  /* boucle principale */
  while (! jdesc->full) {
    int marker = read_marker(stream);
    switch (marker) {
      case 0xffd8:
        break;
      case 0xffe0:
        /* section APP0 */
        read_app0(stream);
        break;
      case 0xfffe:
        /* COM */
        skip_comment(stream);
        break;
      case 0xffdb:
        /* DQT */
        read_dqt(stream, jdesc);
        break;
      case 0xffc0:
        /* SOF0 */
        read_sof(stream, jdesc);
        break;
      case 0xffc4:
        /* DHT */
        read_dht(stream, jdesc);
        break;
      case 0xffda:
        /* SOS */
        read_sos(stream, jdesc);
        break;
      case 0xff00:
        skip_bitstream_until(stream, 0xff);
      default:
        printf("Error : invalid marker %x\n", marker);
        /* TODO free jdesc avant chaque exit */
        exit(1);
    }
  }
  return jdesc;
}

/*
retourne un struct bitstream * pour lire les donnees brutes

Nota: struct bitstream defini dans bitstream.c */
struct bitstream * get_bitstream(const struct  jpeg_desc *jdesc) {
  return jdesc->stream;
}

/* retourne le nom de fichier de l'image ouverte */
char * get_filename(const struct  jpeg_desc *jdesc) {
  return jdesc->filename;
}

/* free le descripteur et ferme le fichier JPEG */
void  close_jpeg(struct  jpeg_desc *jdesc) {
  close_bitstream(jdesc->stream);
  for (uint8_t i=0; i<4; i++) {
    free(jdesc->qtables[i].ptr);
    free_huffman_table(jdesc->huff_table_ac[i]);
    free_huffman_table(jdesc->huff_table_dc[i]);
  }
  free(jdesc->qtables);
  free(jdesc->huff_table_dc);
  free(jdesc->huff_table_ac);
  free(jdesc);
}

/* Entetes DQT pour les tables de quantification */

/* retourne le nombre de tables de quantifications */
uint8_t  get_nb_quantization_tables(const struct  jpeg_desc *jdesc) {
  return jdesc->nb_quantization_tables;
}

/*
retourne un pointeur sur la ieme table de quantification (=tableau de 64 octets non signes)

Nota: allouee par read_jpeg et free par close_jpeg
*/
uint8_t * get_quantization_table (const struct  jpeg_desc *jdesc, uint8_t index ) {
  struct table table = jdesc->qtables[index];
  return table.ptr;
}

/* Entetes DHT pour les tables de Huffman */

/* retourne le nombre de tables de huffman de la composante AC ou DC */
uint8_t  get_nb_huffman_tables(const struct  jpeg_desc *jdesc, enum acdc acdc) {
  if (acdc == AC) {
    return jdesc->nb_huff_ac;
  }
  return jdesc->nb_huff_dc;
}

/*
retourne un pointeur vers la ieme table de Huffman de la compoante AC ou DC

Nota:
1) struct huff_table est defini dans huffman.c
2) allouee par read_jpeg et free par close_jpeg
*/
struct huff_table * get_huffman_table(const struct  jpeg_desc *jdesc, enum acdc acdc, uint8_t index ) {
  if (acdc == AC) {
    return jdesc->huff_table_ac[index];
  }
  return jdesc->huff_table_dc[index];
}

/* SOF0  Frame Header */

/* retourne la dimension dans la direction donnee en nb de pixels */
uint16_t  get_image_size(struct  jpeg_desc *jdesc, enum direction dir) {
  if (dir == DIR_V) {
    return jdesc->hauteur;
  }
  return jdesc->largeur;
}

/*
1 pour noir et blanc
3 pour couleurs
*/
uint8_t get_nb_components(const struct  jpeg_desc *jdesc) {
  return jdesc->nb_comp;
}

/*
retourne l'identifiant de la ieme composante du Frame Header

Nota: i=0 pour Y, 1 pour Cb et 2 pour Cr
*/
uint8_t  get_frame_component_id(const struct  jpeg_desc *jdesc, uint8_t frame_comp_index ) {
  if (frame_comp_index == 0) {
    return jdesc->iCY;
  } else if (frame_comp_index == 1) {
    return jdesc->iCCb;
  }
  return jdesc->iCCr;
}

/*
retourne le facteur d'echantillonage dans la direction voulue pour la ieme composante

Nota: i(ndice) et pas id(entifiant)
*/
uint8_t  get_frame_component_sampling_factor(const struct  jpeg_desc *jdesc, enum direction dir, uint8_t frame_comp_index ) {
  uint8_t sampling_factor;
  if (frame_comp_index == 0) {
    sampling_factor = (dir == DIR_V)?jdesc->SFVY:jdesc->SFHY;
    return sampling_factor;
  } else if (frame_comp_index == 1) {
    sampling_factor = (dir == DIR_V)?jdesc->SFVCb:jdesc->SFHCb;
    return sampling_factor;
  }
  sampling_factor = (dir == DIR_V)?jdesc->SFVCr:jdesc->SFHCr;
  return sampling_factor;
}

/* retourne l'indice de la table de quantification de la ieme composante */
uint8_t  get_frame_component_quant_index(const struct  jpeg_desc *jdesc, uint8_t frame_comp_index ) {
  if (frame_comp_index == 0) {
    return jdesc->iQY;
  } else if (frame_comp_index == 1) {
    return jdesc->iQCb;
  }
  return jdesc->iQCr;
}

/* SOS  Scan Header */

/*
retourne l'identifiant de la ieme composante lue dans le scan

Nota: cette identifiant doit correspondre a un des identifiants lus dans le Frame Header
*/
uint8_t get_scan_component_id(const struct  jpeg_desc *jdesc, uint8_t scan_comp_index) {
  if (scan_comp_index == jdesc->iY) {
    return jdesc->iCY;
  } else if (scan_comp_index == jdesc->iCb) {
    return jdesc->iCCb;
  }
  return jdesc->iCCr;
}

/* retourne l'indice de la table de Huffman (AC ou DC) associee a la ieme composante lue */
uint8_t  get_scan_component_huffman_index(const struct jpeg_desc *jdesc, enum acdc acdc, uint8_t scan_comp_index) {
  uint8_t huff_index;
  if (scan_comp_index == jdesc->iY) {
    huff_index = (acdc == AC)?jdesc->iACY:jdesc->iDCY;
    return huff_index;
  } else if (scan_comp_index == jdesc->iCb) {
    huff_index = (acdc == AC)?jdesc->iACCb:jdesc->iDCCb;
    return huff_index;
  }
  huff_index = (acdc == AC)?jdesc->iACCr:jdesc->iDCCr;
  return huff_index;
}
