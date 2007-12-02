/*
 * Readme
 * This file is part of Hermes
 *
 * Copyright (C) 2008 - Antoine Letouzey antoine.letouzey@gmail.com
 *
 * Hermes is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Hermes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hermes; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 il s'agit d'un logiciel de transfert de video entre un serveur et un client

le serveur filme et envoie la video via une connexion reseau au client.
le client se connecte au serveur, recoit la video et l'affiche.

Le serveur tourne obligatoirement sur psp et se nomme "hermes-serveur"

le client tourne soit sur pc linux soit sur psp

## comment lancer le serveur ?
1) copier le dossier "Serveur" dans le dossier game de la psp
2) lancer le homebrew
3) attendre que la connexion soit prete, 
ATTENTION : le homebrew ne peut utiliser que la premiere configuration reseau
4) brancher la goCam
5) bien noter l'adresse ip affichée.
6) appuyer sur [X]
7) fini normalement la psp filme et attend les connexions

## comment lancer le client sur psp
1) copier le dossier "Client" dans le dossier game de la psp
2) lancer le homebrew
3) Entrer l'adresse IP fournie par le client 
les fleches droite et gauche permettent de changer de numero et haut et bas permettent de changer la valeur du numero courant, BAS = -10, haut = +1
4) appuyer sur [X]
5) Attendre que la connexion se fasse
ATTENTION : le homebrew ne peut utiliser que la premiere configuration reseau
6) fini normalement la psp affiche ce que l'autre console filme

## comment lancer le client sur pc Linux
1) dans le dossier clientPC taper :
./client aaa.bbb.ccc.ddd
en remplaçant aaa.bbb.ccc.ddd par l'adresse ip de la console serveur
2) fini normalement une fenetre s'ouvre et affiche ce que la psp filme.






NOTES :
le frame rate dépend uniquement de la qualité de la connexion.
en cas de pb de chargement de modules psp, c'est que le prog est pas compatible avec la version courante du custom firmware.
Je tourne en 3.40oe.

Les sources sont fournies mais il doit manquer les libs pour la goCam, elles sont dispo dans le nouveau sdk mais je les ai mises dans un dossier a part, il suffit de modifier le makefile en conséquence.



