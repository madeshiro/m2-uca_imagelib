# Projet CVFORAGRICULTURE

## Introduction
Ce projet d’étude a pour objectif d'appliquer les connaissances acquises dans le cadre du traitement d'images, avec un système de désherbage. L'application est développée en C++. Elle permet de détecter et de traiter les adventices présentes dans les cultures pour réduire l'utilisation des produits phytosanitaires.

## Comment lancer le projet

### Pour compiler et exécuter le projet avec cmake:
    
 #### Créer un répertoire de construction :
```bash
    mkdir build
    cd build
```

 #### Compiler le projet :
```bash
    cmake ..
    make
```

#### Exécuter l'application :
L'exécutable c'est : ``` CVFORAGRICULTURE ```. il sera créé dans le répertoire build.

L'application récupère les images dans le répertoire racine data et génère un répertoire contenant touts les masques et les détails des images, ainsi qu'un fichier CSV avec les résultats.

## Auteurs
- Rin Baudelet
- Yorick Geoffre
- Florian Klein
- Clément Guerin

## Références
EUPI-2024 UCA - Banc d’essais pour les systèmes de désherbage alternatifs.

