/*
** Copyright 2007-2023 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#ifndef __SOLVER_H2O_J_DONNEES_ANNEE__
#define __SOLVER_H2O_J_DONNEES_ANNEE__

#define OUI 1
#define NON 0
#define EMERGENCY_SHUT_DOWN 2

#include "h2o_j_donnees_optimisation.h"

/*************************************************************************************************/
/*                    Structure contenant les champs a renseigner par l'appelant */

struct DONNEES_MENSUELLES
{
    /* En entree: seules les donnees ci-dessous doivent etre renseignees par l'appelant apres
       avoir appele H2O_J_Instanciation */
    int NombreDeJoursDuMois; /* A renseigner par l'appelant */
    double TurbineDuMois;    /* A renseigner par l'appelant (c'est le turbine opt du mois) */
    std::vector<double> TurbineMax;      /* A renseigner par l'appelant : 1 valeur par jour */
    std::vector<double> TurbineMin;
    std::vector<double> TurbineCible;    /* A renseigner par l'appelant : 1 valeur par jour */
    /* Les resultats */
    char
      ResultatsValides; /* Vaut:
                           OUI si la solution est exploitable pour le reservoir
                           NON s'il y a eu un probleme dans la resolution
                                                                                                                     EMERGENCY_SHUT_DOWN si la resolution du probleme a donne lieu a une erreur interne
                                                                                                      */
    std::vector<double> Turbine;    /* Resultat a recuperer par l'appelant */

    /******************************************************************************************/
    /* Problemes internes (utilise uniquement par l'optimisation) */
    PROBLEME_HYDRAULIQUE ProblemeHydraulique;
};

#endif
