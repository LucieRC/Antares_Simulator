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
#ifndef __SOLVER_OPTIMISATION_STRUCTURE_PROBLEME_A_RESOUDRE_H__
#define __SOLVER_OPTIMISATION_STRUCTURE_PROBLEME_A_RESOUDRE_H__

#include <vector>
#include <string>
#include "opt_constants.h"

/*--------------------------------------------------------------------------------------*/

namespace Antares::Solver::Optimization {

struct OptimizationOptions
{
    bool useOrtools;
    std::string solverName;
};

}

/* Le probleme a resoudre */
struct PROBLEME_ANTARES_A_RESOUDRE
{
    /* La matrice des contraintes */
    int NombreDeVariables;
    int NombreDeContraintes; /* Il est egal a :
                                 (  NombreDePays contraintes de bilan
                                  + NombreDePays contraintes de reserve
                                  + NombreDeContraintesCouplantes
                                 )* NombreDePasDeTempsDUneJournee
                                  + NombreDePays contraintes de turbine */
    std::string Sens;
    std::vector<int> IndicesDebutDeLigne;
    std::vector<int> NombreDeTermesDesLignes;
    std::vector<double> CoefficientsDeLaMatriceDesContraintes;
    std::vector<int> IndicesColonnes;
    int NombreDeTermesAllouesDansLaMatriceDesContraintes;
    int IncrementDAllocationMatriceDesContraintes;
    int NombreDeTermesDansLaMatriceDesContraintes;
    /* Donnees variables de la matrice des contraintes */
    std::vector<double> CoutQuadratique;
    std::vector<double> CoutLineaire;
    std::vector<int> TypeDeVariable; /* Indicateur du type de variable, il ne doit prendre que les suivantes
                             (voir le fichier spx_constantes_externes.h mais ne jamais utiliser les
                            valeurs explicites des constantes): VARIABLE_FIXE                  ,
                              VARIABLE_BORNEE_DES_DEUX_COTES ,
                              VARIABLE_BORNEE_INFERIEUREMENT ,
                              VARIABLE_BORNEE_SUPERIEUREMENT ,
                              VARIABLE_NON_BORNEE
                                            */
    std::vector<double> Xmin;
    std::vector<double> Xmax;
    std::vector<double> SecondMembre;
    /* Tableau de pointeur a des doubles. Ce tableau est parallele a X, il permet
       de renseigner directement les structures de description du reseau avec les
       resultats contenus dans X */
    std::vector<double*> AdresseOuPlacerLaValeurDesVariablesOptimisees;
    /* Resultat */
    std::vector<double> X;
    /* Tableau de pointeur a des doubles. Ce tableau est parallele a CoutsMarginauxDesContraintes,
       il permet de renseigner directement les structures de description du reseau avec les
       resultats sur les couts marginaux */
    std::vector<double*> AdresseOuPlacerLaValeurDesCoutsMarginaux;
    std::vector<double> CoutsMarginauxDesContraintes;
    /* Tableau de pointeur a des doubles. Ce tableau est parallele a CoutsMarginauxDesContraintes,
       il permet de renseigner directement les structures de description du reseau avec les
       resultats sur les couts reduits */
    std::vector<double*> AdresseOuPlacerLaValeurDesCoutsReduits;
    std::vector<double> CoutsReduits;
    /* En Entree ou en Sortie */
    int ExistenceDUneSolution; /* En sortie, vaut :
                                   OUI_SPX s'il y a une solution,
                                   NON_SPX s'il n'y a pas de solution admissible
                                   SPX_ERREUR_INTERNE si probleme a l'execution (saturation memoire
                                  par exemple), et dans ce cas il n'y a pas de solution
                                   SPX_MATRICE_DE_BASE_SINGULIERE si on n'a pas pu construire de
                                  matrice de base reguliere, et dans ce cas il n'y a pas de solution
                                */

    std::vector<void*> ProblemesSpx;

    std::vector<int> PositionDeLaVariable; /* Vecteur a passer au Simplexe pour recuperer la base optimale */
    std::vector<int> ComplementDeLaBase;   /* Vecteur a passer au Simplexe pour recuperer la base optimale */

    /* Vecteurs de travail pour contruire la matrice des contraintes lineaires */
    std::vector<double> Pi;
    std::vector<int> Colonne;

    /* Nommage des variables & contraintes */
    std::vector<std::string> NomDesVariables;
    std::vector<std::string> NomDesContraintes;

    std::vector<bool> VariablesEntieres; // true = int, false = continuous

    std::vector<int> StatutDesVariables;
    std::vector<int> StatutDesContraintes;

};

#endif /* __SOLVER_OPTIMISATION_STRUCTURE_PROBLEME_A_RESOUDRE_H__ */
