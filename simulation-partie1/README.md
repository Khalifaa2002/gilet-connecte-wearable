# Simulation Wokwi - Partie 1

Maquette ESP32 sans DHT22 : les potentiometres remplacent les capteurs. `potStretch` sur GPIO34 simule l'elongation/respiration; `potTemp` sur GPIO35 simule une temperature de 30 a 42 C. Les boutons Choc, ECG et Annuler sont respectivement sur GPIO18, GPIO19 et GPIO23. Aucune bibliotheque externe n'est requise.

## Logique des alertes

- **Chute** : le bouton Choc simule un impact. Le programme lance un delai de 30 s; sans annulation, il simule l'envoi BLE/SOS. Aucune IMU ne mesure l'immobilite dans cette maquette.
- **ECG** : le bouton ECG simule une anomalie; l'alerte est confirmee apres 6 s. Aucun ECG reel n'est acquis.
- **Thermique** : la pre-alerte jaune est calculee lorsque la hausse estimee depasse 0,1 C/min sur la fenetre de 30 s. La confirmation rouge et le buzzer se declenchent apres 120 s continues a 39 C ou plus. Toute baisse sous 39 C avant confirmation remet le delai a zero. Apres confirmation, l'alerte reste memorisee jusqu'a Annuler.

## Verification rapportee

L'auteur a rapporte avoir maintenu le potentiometre au-dessus de 39 C pendant environ deux minutes, observe la LED rouge et le buzzer, puis appuye sur Annuler et retrouve l'etat normal. Les captures `captures/partie1/alerte-thermique-confirmee.png` et `captures/partie1/alerte-thermique-apres-annulation.png` documentent les deux etats : alerte rouge/buzzer a 42 C, puis retour a l'etat normal apres Annuler. La capture active montre aussi la ligne de confirmation dans le moniteur serie.

Ces valeurs sont simulees et ne correspondent pas a des mesures physiologiques. Les seuils de la maquette illustrent une logique de demonstration; ils ne valident pas un dispositif medical.
