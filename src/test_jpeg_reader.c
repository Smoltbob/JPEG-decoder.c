#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "bitstream.h"
#include "huffman.h"
#include "jpeg_reader.h"

void affiche(struct jpeg_desc* jdesc);

/*
affiche les differents champs du descripteurs jpeg pour une image donnee

Nota: 1) make ./bin/test_jpeg_reader ELEVE=1 utilise notre implementation de jpeg_reader
      2) make ./bin/test_jpeg_reader ELEVE=0 utilise votre implementation de jpeg_reader
*/
int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s fichier.jpeg\n", argv[0]);
        return EXIT_FAILURE;
    }
    if( access( argv[1], F_OK ) == -1 ) {
        fprintf(stderr, "Error : file %s does not exist\n", argv[1]);
        return EXIT_FAILURE;
    }
    struct jpeg_desc* jpeg = read_jpeg(argv[1]);
    affiche(jpeg);
    close_jpeg(jpeg);
}

void affiche_table(uint8_t *table);
void affiche_dqt(struct jpeg_desc* jpeg);
void affiche_dht(struct jpeg_desc* jpeg);
void affiche_sof(struct jpeg_desc* jpeg);
void affiche_sos(struct jpeg_desc* jpeg);

/* affichage des differentes sections */
void affiche(struct jpeg_desc* jpeg)
{
    /* filename */
    printf("***Filename***\n");
    char *filename = get_filename(jpeg);
    printf("filename : %s\n", filename);
    affiche_dqt(jpeg);
    affiche_dht(jpeg);
    affiche_sof(jpeg);
    affiche_sos(jpeg);
}

void affiche_table(uint8_t *table)
{
    for (uint8_t j=0; j<64; j++) {
        if (j%8 == 7) {
            printf("%d\n", table[j]);
        } else {
            printf("%d ", table[j]);
        }
    }
}

void affiche_dqt(struct jpeg_desc* jpeg)
{
    printf("***DQT***\n");
    uint8_t val = get_nb_quantization_tables(jpeg);
    printf("nb_quantization_tables : %d\n", val);
    uint8_t *table;
    for (uint8_t i=0; i<val; i++) {
        table = get_quantization_table(jpeg, i);
        printf("*table %hhu*\n", i);
        affiche_table(table);
    }
}

void affiche_dht(struct jpeg_desc* jpeg)
{
    uint32_t val;
    printf("***DHT***\n");
    val = get_nb_huffman_tables(jpeg, DC);
    printf("nb_huffman_tables DC : %d\n", val);
    val = get_nb_huffman_tables(jpeg, AC);
    printf("nb_huffman_tables AC : %d\n", val);
}

void affiche_sof(struct jpeg_desc* jpeg)
{
    uint8_t val;
    printf("***SOF***\n");
    val = get_nb_components(jpeg);
    printf("hauteur : %d\n", get_image_size(jpeg, DIR_V));
    printf("largeur : %d\n", get_image_size(jpeg, DIR_H));
    printf("nb_comp : %d\n", val);
    printf("frame_comp_id Y : %d\n", get_frame_component_id(jpeg, 0));
    if (val == 3) {
        printf("frame_comp_id Cr : %d\n", get_frame_component_id(jpeg, 1));
        printf("frame_comp_id Cr : %d\n", get_frame_component_id(jpeg, 2));
    }
    printf("sampling_factor (hxv) Y : %dx%d\n",
           get_frame_component_sampling_factor(jpeg, DIR_H, 0), get_frame_component_sampling_factor(jpeg, DIR_V, 0));
    if (val == 3) {
        printf("sampling_factor (hxv) Cb : %dx%d\n",
               get_frame_component_sampling_factor(jpeg, DIR_H, 1), get_frame_component_sampling_factor(jpeg, DIR_V, 1));
        printf("sampling_factor (hxv) Cr : %dx%d\n",
               get_frame_component_sampling_factor(jpeg, DIR_H, 2), get_frame_component_sampling_factor(jpeg, DIR_V, 2));
    }
    printf("quant_index Y : %d\n",
           get_frame_component_quant_index(jpeg, 0));
    if (val == 3) {
        printf("quant_index Cb : %d\n",
               get_frame_component_quant_index(jpeg, 1));
        printf("quant_index Cr : %d\n",
               get_frame_component_quant_index(jpeg, 2));
    }
}

void affiche_sos(struct jpeg_desc* jpeg)
{
    uint8_t val = get_nb_components(jpeg);
    printf("***SOS***\n");
    printf("scan_id Y : %d\n",
           get_scan_component_id(jpeg, 0));
    if (val == 3) {
        printf("scan_id Cb : %d\n",
               get_scan_component_id(jpeg, 1));
        printf("scan_id Cr : %d\n",
               get_scan_component_id(jpeg, 2));
    }
    printf("scan_huff_idx DC Y : %d\n",
           get_scan_component_huffman_index(jpeg, DC, 0));
    if (val == 3) {
        printf("scan_huff_idx DC Cb : %d\n",
               get_scan_component_huffman_index(jpeg, DC, 1));
        printf("scan_huff_idx DC Cr : %d\n",
               get_scan_component_huffman_index(jpeg, DC, 2));
    }
}
