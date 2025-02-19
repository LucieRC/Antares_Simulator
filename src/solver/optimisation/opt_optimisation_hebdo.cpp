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

#include "opt_structure_probleme_a_resoudre.h"

#include "../simulation/simulation.h"
#include "../simulation/sim_extern_variables_globales.h"

#include "opt_fonctions.h"
#include <antares/fatal-error.h>
#include <antares/logs/logs.h>
#include <antares/exception/UnfeasibleProblemError.hpp>

extern "C"
{
#include "spx_fonctions.h"
}

using namespace Antares;
using namespace Antares::Data;

using Antares::Solver::Optimization::OptimizationOptions;

void OPT_OptimisationHebdomadaire(const OptimizationOptions& options,
                                  PROBLEME_HEBDO* pProblemeHebdo,
                                  const AdqPatchParams& adqPatchParams,
                                  Solver::IResultWriter& writer)
{
    if (pProblemeHebdo->TypeDOptimisation == OPTIMISATION_LINEAIRE)
    {
        if (!OPT_PilotageOptimisationLineaire(options, pProblemeHebdo, adqPatchParams, writer))
        {
            logs.error() << "Linear optimization failed";
            throw UnfeasibleProblemError("Linear optimization failed");
        }
    }
    else if (pProblemeHebdo->TypeDOptimisation == OPTIMISATION_QUADRATIQUE)
    {
        OPT_LiberationProblemesSimplexe(options, pProblemeHebdo);
        if (!OPT_PilotageOptimisationQuadratique(pProblemeHebdo))
        {
            logs.error() << "Quadratic optimization failed";
            throw UnfeasibleProblemError("Quadratic optimization failed");
        }
    }
    else
    {
        throw FatalError("Bug: TypeDOptimisation, OPTIMISATION_LINEAIRE ou OPTIMISATION_QUADRATIQUE "
                         "non initialise");
    }
}
