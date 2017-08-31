Projet Décodeur JPEG
====================
L'objectif de ce projet est de réaliser en langage C un décodeur d'images compressées au format JPEG, pour les écrire dans les formats bruts ppm ou pgm.

Pour commencer
--------------
Le projet a été réalisé en deux grosses parties. Premièrement nous avons utilisé les modules bitstream, huffman et jpeg_reader fournis pour réaliser la partie décodage JPEG. Puis nous avons réalisé nous-mêmes ces modules.
Pour différencier les versions enseignants de nos versions des modules, nos modules sont placés dans le dossier src et leurs fichiers objets sont placés dans le dossier `obj`.

### Prérequis
`make` et `clang` pour compiler le projet.
Un visionneur d'images tel qu' `eog` est également utile.

### Compiler le projet
Utiliser `make jpeg2ppm ELEVE=1` pour compiler le projet avec nos modules et `make jpeg2ppm ELEVE=0` pour compiler le projet avec les modules enseignants. Pour passer d'une version à l'autre de l'exécutable, nettoyer le projet avec `make mrproper` avant de recompiler.
Si la compilation a terminé avec succès, le message `Make terminé !` doit s'afficher.

### Utiliser le décodeur
`./bin/jpeg2ppm <chemin vers l'image>` pour décoder une image jpeg.
Une image ppm (image couleur) ou pgm (noir et blanc) du même nom que le fichier d'entrée sera générée dans le même dossier.

Utiliser les tests
-------------------
### Compilation
Nos tests sont en grande partie écrits en C et doivent être compilés.
Pour cela utiliser `make` suivi de ..

* `testex` pour les tests du module extracteur.
* `testppm` pour le module ppm. Ce test génère à la racine du projet une image ppm 8x8 pixels d'un invader aux yeux rouges.
* `test_rgb` pour tester la conversion rgb.
* `test_upsampler` pour tester l'upsampling.
* `test_structure` pour tester nos structures.
* `test_jpeg_reader` pour vérifier que le descripteur est correctement rempli.
* `test_decoder` pour vérifier que les données de l'image Jpeg sont correctement décodées
* `test_mon_bitstream` pour vérifier que toutes les fonctionnalités de `bitstream` sont supportées: lecture de 0 à 32 bits, lecture d'un fichier entier et recherche d'octets dans le flux.

### Scripts
Dans le dossier `scripts_test` se trouvent des scripts qui servent à automatiser ou compléter certains tests, principalement concernant le module bitstream et le décodage des données jpeg (fichier extracteur.c).

#### autotest.sh
Démarre le binaire jpeg2ppm pour chaque fichier jpeg du dossier image. Permet de vérifier rapidement que toutes les images brutes sont générées.

#### test_decoder.sh
Prend une image jpeg en argument. Permet de vérifier que l'image est correctement décodée en comparant les résultats que l'on obtient à ceux fournis par jpeg2blabla.

#### writebinfile.sh
Racourci qui permet de concaténer une suite d'octets à un fichier. Utile pour déboguer le module `bitstream`.
Fichiers notables:

- *bitsaproblemes* est une séquence de bits qui causait un segmentation
        fault avec albert.jpg
- *test_bytestuff* m'a permis de déboguer la gestion du byte stuffing.

#### read_binary.py
Affiche les octets d'un fichier se trouvant dans un certain intervalle d'indexes. Utile quand on sait qu'un fichier est mal décodé à partir d'un certain point.

### cmp_ppm.py
Prend en argument deux fichiers. Leurs octets sont ensuite comparés deux à deux. Si leur différence en valeur absolue est trop grande on considère que les octets diffèrent.

### test_jpeg2ppm.sh
Pour chaque image JPEG contenue dans le dossier images, on génère deux fichiers ppm : un avec jpeg2blabla et un avec jpeg2ppm. Ensuite ces deux fichiers sont comparés avec cmp_ppm.sh. Si 0 octets diffèrent entre les deux on considède que l'image a été correctement décodée.

### Images supplémentaires
Nous avons généré d'autres images avec `gimp` et `vim xxd` pour tester des cas particuliers. Ces images sont dans le dossier `images/test`.

* *Invader_wronghead* : on a remplacé le flag <ff><da> par <da>.
Réponse attendue : [src/jpeg_reader.c] ERROR: Invalid marker, expected 0xff, got 0xda

* *Invader63* : on a supprimé un octet dans le bloc image.
Réponse attendue : l'image est décodée (idem qu'avec Gimp ou EoG).

* *Tricorn* : très grande image.
Réponse attendue : image bien décodée (fichier ppm de 300Mo), en environ 70s (contre 3s pour complexite).

* *Scierie* : encodée en mode progressif.
Réponse attrendue : [src/jpeg_reader.c] ERROR:   application APP mode APP1 is not handled by this decoder (APP0 only)

* *test ixj to kxl.jpg* : pour tester plusieurs échantillonnages (générées avec cjpeg).

* *Carambar* : pour la postérité.

Autres fichiers utiles
----------------------
### Dossier GenieLog
Contient des diagrammes sur le projet : notre modélisation UML ainsi que le graphe d'appels généré par KCachegrind.

Etudiants
---------
* **Benjamin Bouilhac** : conversion RGB, traitement, upsampling et jpeg_reader
* **Jules Simon** : extracteur, ppm et bitstream
* **Mounir Qat** : traitement, iDCT rapide et huffman
