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

#include "../application.h"
#include "../simulation/solver.h"
#include "../simulation/adequacy.h"
#include <antares/benchmarking/DurationCollector.h>
#include <antares/logs/logs.h>

namespace Antares
{
namespace Solver
{
void Application::runSimulationInAdequacyMode()
{
    // Type of the simulation
    typedef Solver::Simulation::ISimulation<Solver::Simulation::Adequacy> SimulationType;
    SimulationType simulation(*pStudy, pSettings, pDurationCollector, *resultWriter);
    simulation.checkWriter();
    simulation.run();

    if (!(pSettings.noOutput || pSettings.tsGeneratorsOnly))
    {
        Benchmarking::Timer timer;
        simulation.writeResults(/*synthesis:*/ true);
        timer.stop();
        pDurationCollector.addDuration("synthesis_export", timer.get_duration());

        this->pOptimizationInfo = simulation.getOptimizationInfo();
    }
}
} // namespace Solver
} // namespace Antares
