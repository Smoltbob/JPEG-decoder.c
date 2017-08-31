#!/bin/bash
# Test si le décodeur de blocs (module extracteur.c) fonctionne bien.
# Syntaxe : ./test_decoder.sh <chemin vers l'image jpeg>

# On récupère le chemin absolu vers la racine du projet
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"

if [ "$#" -ne 1 ]; then
    echo -e "\e[1;34mSyntaxe : ./test_decoder.sh <chemin vers l'image jpeg>\e[0m"
    exit 1
fi

# Chemin vers le fichier
FILE=$1
FILEBLA="${FILE%.*}.blabla"
# Démarage de notre code de test
TEST="$("$DIR"/bin/test_decoder "$FILE" | head -30)"
# Génération du fichier .blabla
"$DIR"/bin/jpeg2blabla "$FILE"
# Lecture du fichier blabla en question
BLABLA="$(more "$FILEBLA" | grep bloc] | head -30)"
# Comparaison des résultats. On ignore les espaces, tabs et la casse.
DIFF="$(diff -q -i -E -Z -b -B <(echo "${TEST}") <(echo "${BLABLA}"))"

echo -e "Fichier : \e[4m $1\e[0m"
if [ "$DIFF" == "" ]
then
    echo -e "\e[1;32mTest OK\e[0m, decoder.c et jpeg2blabla prof donnent le même résultat."
else
    echo -e "\e[1;31mTest KO\e[0m, decodder.c et jpeg2blabla ne donnent pas le même résultat."
fi
