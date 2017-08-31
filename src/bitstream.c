#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdbool.h>

/* Structure :
 * - buffer contient les 8 bits dernièrement lus par fread
 * - idx est un indice sur le buffer (avant idx : les valeurs qui ont déjà
 *   été retournées. après idx : les valeurs que l'on va retourner)
 *   Si idx = 8 on appelle de nouveau fread pour charger un nouvel octet dans
 *   le buffer.
 * - end_of_stream : passe à true à la fin du stream.
 * - ff flag pour détecter le xFF x00
 */
struct bitstream {
    FILE* fichier;
    uint8_t buffer;
    uint8_t idx;
    bool end_of_stream;
    bool ff;
};

/* Initialise la structure bitstream en ouvrant un fichier */
struct bitstream *create_bitstream(const char *filename)
{
    FILE *fichier = fopen(filename, "r");
    struct bitstream* stream = malloc(sizeof(struct bitstream));
    stream->fichier = fichier;
    stream->buffer = 0;
    stream->idx = 8;
    stream->end_of_stream = false;
    stream->ff = false;
    return stream;
}

void close_bitstream(struct bitstream *stream)
{
    fclose(stream->fichier);
    free(stream);
}

uint8_t check_byte_stuff(struct bitstream* stream)
{
    /* Cas 1 : flux = xxx FF xxx */
    if (stream->buffer == 0xFF)
        stream->ff = true;
    else if (stream->ff == true && stream->buffer == 0x00) {
        stream->ff = false;
        return 1;
    } else
        stream->ff = false;
    return 0;
}


uint8_t readfrombyte(struct bitstream* stream, uint8_t bits, uint8_t *val, bool discard_bs)
{
    uint8_t nbread = 0;
    /* Si les bits à lire ne sont pas du tout dans le buffer */
    if (stream->idx == 8) {
        stream->idx = 0;
        uint8_t res = fread(&(stream->buffer), 1, 1, stream->fichier);
        uint8_t res2 = 1;
        if (res != 1)
            stream->end_of_stream = true;
        /* Si il y a un FF 00 on veut ignorer le 00 que l'on vient de lire */
        if (check_byte_stuff(stream) == 1 && discard_bs == true) {
            res2 = fread(&(stream->buffer), 1, 1, stream->fichier);
            if (res2 != 1)
                stream->end_of_stream = true;
        }
        /* On vérifie que les lectures étaient correctes */
        if ((res == 1 || res2 == 1) && (stream->end_of_stream == false))
            nbread += readfrombyte(stream, bits, val, discard_bs);
    }
    /* Sinon si les bits sont entièrement dans le buffer */
    else if (bits <= 8 - stream->idx) {
        /* Masque pour les poids forts de l'octet (pour ne pas tenir compte
         * de ce qui a été déjà lu) */
        uint8_t shiftG = 0xFF >> stream->idx;
        /* Masque pour les poids faibles pour ne pas lire trop de bits*/
        uint8_t shiftD = 0xFF << (8 - bits - stream->idx);
        *val = (stream->buffer & (shiftG & shiftD)) >> (8 - bits - stream->idx);
        stream->idx += bits;
        nbread += bits;
    }
    /* Sinon les bits sont partiellement dans le buffer */
    else {
        /* Lire la partie intéressante du buffer */
        uint8_t bitsbuff = 8 - stream->idx;
        uint8_t val1 = 0;
        nbread += readfrombyte(stream, bitsbuff, &val1, discard_bs);
        /* bits restants à lire */
        bits -= bitsbuff;
        uint8_t val2 = 0;
        nbread += readfrombyte(stream, bits, &val2, discard_bs);
        /* Concaténation des deux valeurs */
        *val = (val1 << bits) | val2;
    }
    return nbread;
}



/* Lit jusqu'à 32 bits d'un coup */
//uint8_t readbits(FILE *file, uint8_t bits, uint32_t *val, uint8_t discard_bs)
uint8_t read_bitstream(struct bitstream* stream, uint8_t bits, uint32_t *val, bool discard_bs)
{
    if (stream->end_of_stream == true)
        return 0;
    uint8_t byte = 0;
    uint8_t nboctets = bits / 8;
    uint8_t nbread = 0;
    *val = 0;
    if (bits % 8 != 0) {
        uint8_t res = readfrombyte(stream, bits % 8, &byte, discard_bs);
        if (res == bits % 8) {
            nbread += res;
            *val |= (byte << nboctets*8);
        }
    }
    for (uint8_t i = 0; i < nboctets; i++) {
        uint8_t res = readfrombyte(stream, 8, &byte, discard_bs);
        if (res == 8) {
            nbread += res;
            *val |= (byte << (i*8));
        }
    }
    /* Passage en big endian.
     * rotationner le + grand groupe de bits de taile multiple de 8
     * pour les convertir en big endian en utilisant htonl,
     * puis ajouter en poids fort les anciens bits de poids faible
     * qui n'ont pas été rotationnés */
    /* Formule généralisée :
     * *val = (htonl(*val & (0xFFFFFFFF  >> (32 - 8*nboctets))) >> \
     * (32 - 8*nboctets)) | (*val & (0xFFFFFFFF << nboctets*8))
     * */
    switch(nboctets) {
    case 4:
        *val = htonl(*val);
        break;
    case 3:
        *val = (htonl(*val & 0x00FFFFFF) >> 8) | (*val & 0xFF000000);
        break;
    case 2:
        *val = (htonl(*val & 0x0000FFFF) >> 16) | (*val & 0xFFFF0000);
        break;
    }
    return nbread;
}


/* Cherche le premier octet égal à byte sur une adresse multiple de 8.
 * Le flux est positionné juste avant cet octet. Par défaut on ignore pas
 * le byte stuffing (ie si il y a FF 00 on lit FF 00) */
void skip_bitstream_until(struct bitstream* stream, uint8_t byte)
{
    uint32_t val = 0;
    do {
        val = 0;
        read_bitstream(stream, 8, &val, false);
    } while (val != byte && stream->end_of_stream == false);
    if (stream->end_of_stream == false)
        fseek(stream->fichier, -1, SEEK_CUR);
}

/* Permet de regarder si il reste des bits à lire */
bool end_of_bitstream(struct bitstream *stream)
{
    return stream->end_of_stream;
}
