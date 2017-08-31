#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
/* Module contenant des fonctions liées à la sécurité du projet. */

/* Vérifie que l'extension d'un fichier est correcte */
bool check_extension(const char* filename, char* extension)
{
    int8_t idx = strlen(filename) - 1;
    int8_t j = strlen(extension) - 1;
    /* On lit la chaîne depuis la fin pour chercher le premier point
     * (46 en ascii) */
    while (idx >= 0 && filename[idx] != 46) {
        if (filename[idx] != extension[j])
            return false;
        idx --;
        j --;
    }
    return true;
}
