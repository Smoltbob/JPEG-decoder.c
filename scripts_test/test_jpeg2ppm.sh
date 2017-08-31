#!/bin/bash
# Ce script génère les images brutes en utilisant jpeg2blabla et jpeg2pmm.
# Les deux versions des images obtenues sont ensuite comparées avec le script
# cmp_ppm.py qui lit deux fichiers en hexadécimal et compare deux à deux leurs
# octets. Si peu d'octets diffèrent alors le décodage de l'image est correct.


# On récupère le chemin absolu vers la racine du projet
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"

echo -e "\e[1;36mAppel de jpeg2blabla\e[0m"
for file in $(pwd)/images/*.jp*g
# On génère tous les ppm avec jpeg2blabla
do
    "$DIR"/bin/jpeg2blabla "$file"
done
# On renomme les fichiers crées
for file in $(pwd)/images/*.p*m
do
    mv "$file" "$file"_blabla
done
echo -e "\e[1;36mAppel de jpeg2ppm\e[0m"
# On génère les ppm avec notre code
for file in $(pwd)/images/*.jp*g
# On génère tous les ppm avec jpeg2blabla
do
    "$DIR"/bin/jpeg2ppm "$file"
done
echo -e "\e[1;36mComparaison des deux versions\e[0m"
# On compare les résultats deux à deux
for file in $(pwd)/images/*.p*m
do
    echo -e "\e[1;34mImage : $file\e[0m"
    "$DIR"/scripts_test/cmp_ppm.py "$file" "$file"_blabla
    rm "$file"
    rm "$file"_blabla
done
echo -e "\e[1;32mTest terminé.\e[0m"
