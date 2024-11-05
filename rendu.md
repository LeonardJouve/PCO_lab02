# Hopital concurrent

Auteurs : Christophe Künzli, Léonard Jouve

## Introduction au problème

On souhaite au travers d'une application de gestion d'inventaire pour des hopitaux gérer des accès concurrents à des ressources partagées entre threads. Pour celà, nous allons utilier des mutex afin de faire de l'exclusion mutuelle.

## Choix d'implémentation

### Seller
Nous avons ajouté le mutex responsable de limiter les accès concurrents aux ressources critiques de chaque instance des objet héritants de Seller. 

### Ambulance
Dans la méthode `sendPatient`, nous avons choisi d'essayer d'envoyer le nombre maximum possible de patients aux hopitaux. La méthode `send` de Hopital nous renvoie le nombre de patients acceptés.

Il n'y a pas de problème de concurrence dans Ambulance, car ses ressources internes et méthodes d'une instance sont accédées uniquement par cette même instance.

### Clinic
La méthode `request` doit uniquement gérer des items de type PatientHealed demandés par les hopitaux.
On limite la quantité demandée à la quantité de patients guéris dans la Clinic.
Cette méthode a une section critique car elle accède et modifie les stocks et l'argent de la clinique et qu'elle est appelée par les hopitaux qui sont executés par d'autres threads.

Dans la méthode `orderRessources`, la clinic commande les outils nécéssaires au soin d'un patient incluant le patient lui même si elle n'en possède pas déja un. 
Cette méthode a également une section critique lors des accès à money et stocks. Ses accès sont donc protégés par son mutex.

La méthode `treatPatient` consomme les ressources nécéssaires au soin d'un patient, met à jour les stocks et l'argent en payant un docteur.
Les accès concurrents sont contrôlés par le mutex.

### Hopital

### Supplier

### Fin d'execution

## Tests effectués

Nous avons testé que notre implémentation arrive correctement à trouver la séquence triée avec différents seeds, tailles et nombre de threads. \
Nous avons aussi vérifié que tous  les threads arrêtaient bien de calculer une fois que la séquence a été trouvée par un autre thread.