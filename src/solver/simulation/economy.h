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
#ifndef __SOLVER_SIMULATION_ECONOMY_H__
#define __SOLVER_SIMULATION_ECONOMY_H__

#include "../variable/variable.h"
#include "../variable/economy/all.h"
#include "../variable/state.h"

#include "../optimisation/base_weekly_optimization.h"
#include "base_post_process.h"

#include "solver.h" // for definition of type yearRandomNumbers
#include "antares/infoCollection/StudyInfoCollector.h"
#include "opt_time_writer.h"

namespace Antares::Solver::Simulation
{
class Economy
{
public:
    //! Name of the type of simulation
    static const char* Name()
    {
        return "economy";
    }

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Constructor
    **
    ** \param study The current study
    */
    Economy(Data::Study& study, IResultWriter& resultWriter);
    //! Destructor
    ~Economy() = default;
    //@}

    Benchmarking::OptimizationInfo getOptimizationInfo() const;

public:
    //! Current study
    Data::Study& study;
    //! All variables
    Solver::Variable::Economy::AllVariables variables;
    //! Prepro only
    bool preproOnly;

protected:
    void setNbPerformedYearsInParallel(uint nbMaxPerformedYearsInParallel);

    bool simulationBegin();

    bool year(Progression::Task& progression,
              Variable::State& state,
              uint numSpace,
              yearRandomNumbers& randomForYear,
              std::list<uint>& failedWeekList,
              bool isFirstPerformedYearOfSimulation,
              const HYDRO_VENTILATION_RESULTS&,
              OptimizationStatisticsWriter& optWriter);

    void incrementProgression(Progression::Task& progression);

    void simulationEnd();

    /*!
    ** \brief Prepare clusters in 'must-run' mode
    */
    void prepareClustersInMustRunMode(uint numSpace, uint year);

    void initializeState(Variable::State& state, uint numSpace);

private:
    uint pNbWeeks;
    uint pStartTime;
    uint pNbMaxPerformedYearsInParallel;
    std::vector<PROBLEME_HEBDO> pProblemesHebdo;
    std::vector<std::unique_ptr<Antares::Solver::Optimization::WeeklyOptimization>> weeklyOptProblems_;
    std::vector<std::unique_ptr<interfacePostProcessList>> postProcessesList_;
    IResultWriter& resultWriter;
}; // class Economy

} // namespace Antares::Solver::Simulation

#endif // __SOLVER_SIMULATION_ECONOMY_H__
