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

#include <yuni/yuni.h>
#include <antares/study/study.h>
#include "simulation.h"
#include "sim_structure_donnees.h"
#include "sim_structure_probleme_economique.h"
#include "sim_extern_variables_globales.h"

using namespace Antares;

static void AllocateResultsForEconomicMode(const Data::Study& study)
{
    transitMoyenInterconnexionsRecalculQuadratique.resize(study.runtime->interconnectionsCount());

    for (uint i = 0; i != study.runtime->interconnectionsCount(); i++)
        transitMoyenInterconnexionsRecalculQuadratique[i].assign(HOURS_PER_YEAR, 0.);
}

void SIM_AllocationTableaux(const Data::Study& study)
{
    AllocateResultsForEconomicMode(study);
}
