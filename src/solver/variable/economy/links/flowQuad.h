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
#ifndef __SOLVER_VARIABLE_ECONOMY_FlowQuad_H__
#define __SOLVER_VARIABLE_ECONOMY_FlowQuad_H__

#include "../../variable.h"

namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace Economy
{
struct VCardFlowQuad
{
    //! Caption
    static std::string Caption()
    {
        return "FLOW QUAD.";
    }
    //! Unit
    static std::string Unit()
    {
        return "MWh";
    }

    //! The short description of the variable
    static std::string Description()
    {
        return "Flow (quad.)";
    }

    //! The expecte results
    typedef Results<R::AllYears::Raw< // Raw values
      >>
      ResultsType;

    enum
    {
        //! Data Level
        categoryDataLevel = Category::link,
        //! File level (provided by the type of the results)
        categoryFileLevel = ResultsType::categoryFile & (Category::id | Category::va),
        //! Precision (views)
        precision = Category::all,
        //! Indentation (GUI)
        nodeDepthForGUI = +0,
        //! Decimal precision
        decimal = 0,
        //! Number of columns used by the variable (One ResultsType per column)
        columnCount = 1,
        //! The Spatial aggregation
        spatialAggregate = Category::spatialAggregateSum,
        spatialAggregateMode = Category::spatialAggregateEachYear,
        spatialAggregatePostProcessing = 0,
        //! Intermediate values
        hasIntermediateValues = 1,
        //! Can this variable be non applicable (0 : no, 1 : yes)
        isPossiblyNonApplicable = 0,
    };

    typedef IntermediateValues IntermediateValuesType;

}; // class VCard

/*!
** \brief Marginal FlowQuad
*/
template<class NextT = Container::EndOfList>
class FlowQuad : public Variable::IVariable<FlowQuad<NextT>, NextT, VCardFlowQuad>
{
public:
    //! Type of the next static variable
    typedef NextT NextType;
    //! VCard
    typedef VCardFlowQuad VCardType;
    //! Ancestor
    typedef Variable::IVariable<FlowQuad<NextT>, NextT, VCardType> AncestorType;

    //! List of expected results
    typedef typename VCardType::ResultsType ResultsType;

    typedef VariableAccessor<ResultsType, VCardType::columnCount> VariableAccessorType;

    enum
    {
        //! How many items have we got
        count = 1 + NextT::count,
    };

    template<int CDataLevel, int CFile>
    struct Statistics
    {
        enum
        {
            count
            = ((VCardType::categoryDataLevel & CDataLevel && VCardType::categoryFileLevel & CFile)
                 ? (NextType::template Statistics<CDataLevel, CFile>::count
                    + VCardType::columnCount * ResultsType::count)
                 : NextType::template Statistics<CDataLevel, CFile>::count),
        };
    };

public:
    ~FlowQuad()
    {
    }

    void initializeFromStudy(Data::Study& study)
    {
        // Average on all years
        pNbHours = study.runtime->rangeLimits.hour[Data::rangeEnd] + 1;
        AncestorType::pResults.initializeFromStudy(study);
        AncestorType::pResults.reset();

        // Intermediate values
        pValuesForTheCurrentYear.initializeFromStudy(study);

        // Next
        NextType::initializeFromStudy(study);
    }

    void initializeFromArea(Data::Study* study, Data::Area* area)
    {
        // Next
        NextType::initializeFromArea(study, area);
    }

    void initializeFromAreaLink(Data::Study* study, Data::AreaLink* link)
    {
        assert(link && "invalid interconnection");
        pLinkGlobalIndex = link->index;
        // Next
        NextType::initializeFromAreaLink(study, link);
    }

    void simulationBegin()
    {
        pValuesForTheCurrentYear.reset();

        // Next
        NextType::simulationBegin();
    }

    void simulationEnd()
    {
        // Flow assessed over all MC years (linear)
        (void)::memcpy(
          pValuesForTheCurrentYear.hour,
          transitMoyenInterconnexionsRecalculQuadratique[pLinkGlobalIndex].data(),
          sizeof(double) * pNbHours);

        // Compute all statistics for the current year (daily,weekly,monthly)
        pValuesForTheCurrentYear.computeStatisticsForTheCurrentYear();
        // Merge all those values with the global results
        AncestorType::pResults.merge(0, pValuesForTheCurrentYear);

        // Next
        NextType::simulationEnd();
    }

    void yearBegin(uint year, unsigned int numSpace)
    {
        // Next variable
        NextType::yearBegin(year, numSpace);
    }

    void yearEndBuild(State& state, unsigned int year)
    {
        // Next variable
        NextType::yearEndBuild(state, year);
    }

    void yearEnd(uint year, unsigned int numSpace)
    {
        // Next variable
        NextType::yearEnd(year, numSpace);
    }

    void computeSummary(std::map<unsigned int, unsigned int>& numSpaceToYear,
                        unsigned int nbYearsForCurrentSummary)
    {
        // Next variable
        NextType::computeSummary(numSpaceToYear, nbYearsForCurrentSummary);
    }

    void hourBegin(uint hourInTheYear)
    {
        // Next variable
        NextType::hourBegin(hourInTheYear);
    }

    void hourForEachArea(State& state, unsigned int numSpace)
    {
        // Next variable
        NextType::hourForEachArea(state, numSpace);
    }

    void hourForEachLink(State& state, unsigned int numSpace)
    {
        // Next item in the list
        NextType::hourForEachLink(state, numSpace);
    }

    void buildDigest(SurveyResults& results, int digestLevel, int dataLevel) const
    {
        if (dataLevel & Category::link)
        {
            if (digestLevel & Category::digestFlowQuad)
            {
                results.data.matrix
                  .entry[results.data.link->from->index][results.data.link->with->index]
                  = AncestorType::pResults.rawdata.allYears;
                results.data.matrix
                  .entry[results.data.link->with->index][results.data.link->from->index]
                  = -AncestorType::pResults.rawdata.allYears;
            }
        }
        // Next
        NextType::buildDigest(results, digestLevel, dataLevel);
    }

    Antares::Memory::Stored<double>::ConstReturnType retrieveRawHourlyValuesForCurrentYear(
      uint,
      uint) const
    {
        return pValuesForTheCurrentYear.hour;
    }

    void localBuildAnnualSurveyReport(SurveyResults& results,
                                      int fileLevel,
                                      int precision,
                                      uint) const
    {
        // Initializing external pointer on current variable non applicable status
        results.isCurrentVarNA = AncestorType::isNonApplicable;

        if (AncestorType::isPrinted[0])
        {
            // Write the data for the current year
            results.variableCaption = VCardType::Caption();
            results.variableUnit = VCardType::Unit();
            pValuesForTheCurrentYear.template buildAnnualSurveyReport<VCardType>(
              results, fileLevel, precision);
        }
    }

private:
    uint pLinkGlobalIndex;
    uint pNbHours;
    //! Intermediate values for each year
    typename VCardType::IntermediateValuesType pValuesForTheCurrentYear;

}; // class FlowQuad

} // namespace Economy
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_ECONOMY_FlowQuad_H__
