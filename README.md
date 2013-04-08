Projet annuel M1 SSI 2012-2013 : chat sécurisé
==============================================

Présentation
-----------

Développement d’un système permettant à plusieurs utilisateurs de s’authentifier et de communiquer de manière sécurisée :
* gestion de la création et de la suppression d’un compte utilisateur ;
* création par un utilisateur d’une salle de discussion privée ;
* ajout et suppression d’un utilisateur autorisé dans une salle privée
* confidentialité, intégrité et authentification sur les messages échangés ;
* non répudiation des messages ;
* création d’une autorité de certification ;
* demande de certificat pour l’accès à un salon privé et la communication sécurisée.

Prérequis
--------

Paquets à installer (ubuntu >= 12.04) :
* git
* cmake
* libglib2.0-dev
* libgtk-3-dev
* libgee-dev
* libssl-dev
* libsqlite3-dev

Manuel d'utilisation
--------------------

### Compilation du projet

La compilation du projet se fait en deux temps :

    $ mkdir src/build/
    $ cd src/build/
    $ cmake ..
    $ make

### Utilisation du serveur de chat

    $ cd src/build/server/
    $ ./server adresse port

### Utilisation du client de chat

    $ cd src/client/
    $ ./Client

### Utilisation du serveur de chat sécurisé
    
    $ cp certifs/bavardage* src/build/serversec/
    $ cd src/build/serversec/
    $ ./secure_server adresse port

### Utilisation du client de chat sécurisé

En construction