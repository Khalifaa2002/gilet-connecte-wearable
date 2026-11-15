# Gilet connecté pour sportif de haut niveau

Projet d’ingénierie Wearable Technology : conception système d’un gilet connecté (Smart Textile, Edge AI et télécommunications RF), avec deux démonstrations Wokwi complémentaires.

## Livrable principal

- Rapport d’ingénierie final retenu : [Rapport_Gilet_Connecte_Wearable.pdf](rapport/Rapport_Gilet_Connecte_Wearable.pdf), 12 pages.
- Le rapport couvre les sept rubriques du cahier des charges : fiche/cahier des charges, architecture, instrumentation, Edge AI, télécoms/antenne, énergie et innovation personnelle.
- Les valeurs de conception, calculées ou simulées sont signalées comme telles. Aucune validation physiologique ni mesure sur prototype textile réel n’est revendiquée.

## Contenu du dossier

- simulation-partie1/ : code, schéma Wokwi, dépendances et guide de la maquette d’alertes locales.
- simulation-partie2/ : code, schéma Wokwi et bibliothèques de la maquette de télémétrie MQTT.
- captures/partie1/ : captures finales des essais d’alertes locales.
- captures/partie2/ : captures finales des essais normal, pouls élevé, température élevée et réception MQTT; une capture combinée est conservée comme essai complémentaire.
- rapport/ : le seul rapport PDF à remettre.

## Partie 1 — alertes locales dans Wokwi

### Maquette et logique

La maquette utilise un ESP32, deux potentiomètres (élongation/respiration simulée sur GPIO34 et température simulée sur GPIO35), les boutons Choc, ECG et Annuler, trois LEDs avec résistances et un buzzer. Aucun DHT22 n’est utilisé dans cette partie.

- Choc : le bouton simule l’impact; après le délai de 30 s sans annulation, le programme simule l’envoi BLE/SOS. La maquette ne possède pas d’IMU et ne détecte donc pas réellement une immobilité.
- ECG : le bouton simule une anomalie; l’alerte locale est confirmée après environ 6 s. Aucun signal ECG réel n’est acquis.
- Thermique : pré-alerte jaune selon la pente simulée; alerte rouge et buzzer après 120 s continuellement à au moins 39 °C. Une baisse sous 39 °C avant confirmation remet le délai à zéro. Après confirmation, l’alerte reste mémorisée jusqu’à Annuler.

### Résultats documentés

Les essais rapportés par l’auteur montrent l’état normal, les alertes chute et ECG simulées, l’alerte thermique confirmée après environ deux minutes, puis le retour à l’état normal après Annuler. Les captures correspondantes sont dans captures/partie1. Ces scénarios vérifient la logique du programme dans Wokwi, pas les capteurs ou les performances d’un vêtement réel.

## Partie 2 — télémétrie et MQTT dans Wokwi

### Maquette et configuration

La maquette utilise un ESP32, un potentiomètre sur GPIO34 pour simuler un pouls de 40 à 180 bpm, un DHT22 sur GPIO15 pour simuler température et humidité, des LEDs et un buzzer. Le code lit les valeurs toutes les deux secondes et publie en MQTT.

- Seuils de démonstration : pouls supérieur à 140 bpm; température égale ou supérieure à 39 °C.
- États visibles : LED verte en état normal, jaune pour pouls élevé, rouge pour température élevée; buzzer si une alarme est active.
- Bibliothèques : DHT sensor library for ESPx et PubSubClient (voir simulation-partie2/libraries.txt).
- Wi-Fi de simulation : Wokwi-GUEST. Broker public : broker.hivemq.com, port 1883. Topic : wearable/esprit/khalifa-demo-742/telemetry.
- Le JSON contient pulse, temperature, humidity, alarm et sensor_ok. sensor_ok:true signifie uniquement que le DHT22 simulé a fourni des valeurs numériques exploitables.

### Résultats documentés

| Essai | Valeurs visibles | Résultat observé dans les captures |
|---|---|---|
| État normal | 80 bpm; 31,4 °C | alarm:false; sensor_ok:true; LED verte |
| Pouls élevé seul | 167 bpm; 32 °C | alarm:true; sensor_ok:true; alerte pouls |
| Température élevée seule | 80 bpm; 40,1 °C | alarm:true; sensor_ok:true; alerte thermique |
| Réception MQTT | topic wearable/esprit/khalifa-demo-742/telemetry | JSON reçu avec les cinq champs |
| Essai complémentaire combiné | pouls et température dépassent leurs seuils | alarm:true; deux conditions anormales |

Les captures sont classées dans captures/partie2. Les boutons Choc, ECG et Annuler qui apparaissent dans le diagramme de cette maquette ne sont pas utilisés par le code MQTT; leur logique est démontrée en Partie 1.

## Étapes réalisées

1. Définition de l’architecture et des seuils de démonstration, en séparant les deux objectifs de simulation.
2. Construction de la maquette Partie 1 dans Wokwi, raccordement des potentiomètres, boutons, LEDs et buzzer, puis observation des scénarios normal, chute/ECG simulés et thermique avec annulation.
3. Construction de la maquette Partie 2, raccordement du potentiomètre de pouls au GPIO34 et du DHT22 au GPIO15, ajout des bibliothèques et intégration des alertes locales.
4. Ajout de la connexion Wi-Fi Wokwi et de la publication MQTT; vérification visuelle de la réception sur le client Web HiveMQ avec le topic prévu et le champ sensor_ok.
5. Classement des captures finales par partie et intégration des résultats principaux au rapport final.
6. Préparation du dossier local avec le rapport retenu, les deux maquettes, leurs dépendances, les guides et les captures sélectionnées.

Les résultats de simulation sont des démonstrations rapportées et capturées dans Wokwi/HiveMQ; ce README ne prétend pas qu’une nouvelle simulation a été exécutée pendant la mise en forme du dossier.

## Limites et interprétation

- Les entrées sont synthétiques : le potentiomètre ne mesure pas un rythme cardiaque; le DHT22 Wokwi ne mesure pas une température corporelle.
- Le CNN/SVM décrit dans le rapport est une architecture cible; aucun modèle n’a été entraîné ni exécuté dans ces maquettes.
- L’indice d’asymétrie de foulée est exploratoire, non validé et non simulé.
- L’antenne, l’autonomie, les mesures ECG et les performances Edge AI restent des estimations de conception à valider sur matériel/banc approprié.
- MQTT utilise un broker public sans chiffrement dans cette démonstration. Ne publier que des données synthétiques, jamais des données de santé personnelles.

## Dépôt GitHub et remise

Dépôt communiqué pour le projet : [github.com/Khalifaa2002/gilet-connecte-wearable](https://github.com/Khalifaa2002/gilet-connecte-wearable).

Avant la remise, vérifier que la branche par défaut du dépôt GitHub contient le rapport final, les deux dossiers de simulation, les captures finales et ce README. Pour un dépôt privé, donner accès au professeur.
