#!/bin/bash
# Lance la conversion pour toutes les images contenues dans le dossier images

# On récupère le chemin absolu vers la racine du projet
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )"
echo "$DIR"
echo -e "\e[1;34mDébut du test ....\e[0m"
for file in "$DIR"/images/*.jp*g
do
    echo $file
    "$DIR"/bin/jpeg2ppm "$file"
done
echo -e "\e[1;32mTest terminé, \e[0mregarder les résultats dans le dossier images."
