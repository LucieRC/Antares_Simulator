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
#pragma once

#include "../variable.h"

namespace Antares::Solver::Variable::Economy
{
struct VCardSTstorageCashFlowByCluster
{
    //! Caption
    static std::string Caption()
    {
        return "STS Cashflow By Cluster";
    }
    //! Unit
    static std::string Unit()
    {
        return "CashFlow - Euro";
    }

    //! The short description of the variable
    static std::string Description()
    {
        return "Cash Flow by short term storage";
    }

    //! The expecte results
    typedef Results<R::AllYears::Average< // The average values throughout all years
      >>
      ResultsType;

    //! The VCard to look for for calculating spatial aggregates
    typedef VCardSTstorageCashFlowByCluster VCardForSpatialAggregate;

    enum
    {
        //! Data Level
        categoryDataLevel = Category::area,
        //! File level (provided by the type of the results)
        categoryFileLevel = ResultsType::categoryFile & Category::de_sts,
        //! Precision (views)
        precision = Category::all,
        //! Indentation (GUI)
        nodeDepthForGUI = +0,
        //! Decimal precision
        decimal = 0,
        //! Number of columns used by the variable
        columnCount = Category::dynamicColumns,
        //! The Spatial aggregation
        spatialAggregate = Category::spatialAggregateSum,
        spatialAggregateMode = Category::spatialAggregateEachYear,
        spatialAggregatePostProcessing = 0,
        //! Intermediate values
        hasIntermediateValues = 1,
        //! Can this variable be non applicable (0 : no, 1 : yes)
        isPossiblyNonApplicable = 0,
    };

    typedef IntermediateValues IntermediateValuesDeepType;
    typedef IntermediateValues* IntermediateValuesBaseType;
    typedef IntermediateValuesBaseType* IntermediateValuesType;

}; // class VCard

/*!
** \brief Energy generated by short term storage clusters
*/
template<class NextT = Container::EndOfList>
class STstorageCashFlowByCluster : public Variable::IVariable<STstorageCashFlowByCluster<NextT>,
                                                               NextT,
                                                               VCardSTstorageCashFlowByCluster>
{
public:
    //! Type of the next static variable
    typedef NextT NextType;
    //! VCard
    typedef VCardSTstorageCashFlowByCluster VCardType;
    //! Ancestor
    typedef Variable::IVariable<STstorageCashFlowByCluster<NextT>, NextT, VCardType> AncestorType;

    //! List of expected results
    typedef typename VCardType::ResultsType ResultsType;

    typedef VariableAccessor<ResultsType, VCardType::columnCount> VariableAccessorType;

    static constexpr int count = 1 + NextT::count;

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
    STstorageCashFlowByCluster() = default;

    ~STstorageCashFlowByCluster()
    {
        for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
            delete[] pValuesForTheCurrentYear[numSpace];
        delete[] pValuesForTheCurrentYear;
    }

    void initializeFromArea(Data::Study* study, Data::Area* area)
    {
        // Get the number of years in parallel
        pNbYearsParallel = study->maxNbYearsInParallel;
        pValuesForTheCurrentYear = new VCardType::IntermediateValuesBaseType[pNbYearsParallel];

        // Get the area
        nbClusters_ = area->shortTermStorage.count();
        if (nbClusters_)
        {
            AncestorType::pResults.resize(nbClusters_);

            for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
                pValuesForTheCurrentYear[numSpace]
                  = new VCardType::IntermediateValuesDeepType[nbClusters_];

            for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
                for (unsigned int i = 0; i != nbClusters_; ++i)
                    pValuesForTheCurrentYear[numSpace][i].initializeFromStudy(*study);

            for (unsigned int i = 0; i != nbClusters_; ++i)
            {
                AncestorType::pResults[i].initializeFromStudy(*study);
                AncestorType::pResults[i].reset();
            }
        }
        else
        {
            for (unsigned int numSpace = 0; numSpace < pNbYearsParallel; numSpace++)
            {
                pValuesForTheCurrentYear[numSpace] = nullptr;
            }

            AncestorType::pResults.clear();
        }
        // Next
        NextType::initializeFromArea(study, area);
    }

    size_t getMaxNumberColumns() const
    {
        return nbClusters_ * ResultsType::count;
    }

    void yearBegin(unsigned int year, unsigned int numSpace)
    {
        // Reset the values for the current year
        for (unsigned int clusterIndex = 0; clusterIndex != nbClusters_; ++clusterIndex)
        {
            pValuesForTheCurrentYear[numSpace][clusterIndex].reset();
        }
        // Next variable
        NextType::yearBegin(year, numSpace);
    }

    void yearEnd(unsigned int year, unsigned int numSpace)
    {
        for (unsigned int clusterIndex = 0; clusterIndex < nbClusters_; ++clusterIndex)
        {
            // Compute all statistics from hourly results for the current year (daily, weekly, monthly, ...)
            pValuesForTheCurrentYear[numSpace][clusterIndex].computeStatisticsForTheCurrentYear();
        }
        // Next variable
        NextType::yearEnd(year, numSpace);
    }

    void computeSummary(std::map<unsigned int, unsigned int>& numSpaceToYear,
                        unsigned int nbYearsForCurrentSummary)
    {
        for (unsigned int numSpace = 0; numSpace < nbYearsForCurrentSummary; ++numSpace)
        {
            for (unsigned int clusterIndex = 0; clusterIndex < nbClusters_; ++clusterIndex)
            {
                // Merge all those values with the global results
                AncestorType::pResults[clusterIndex].merge(numSpaceToYear[numSpace],
                                                pValuesForTheCurrentYear[numSpace][clusterIndex]);
            }
        }

        // Next variable
        NextType::computeSummary(numSpaceToYear, nbYearsForCurrentSummary);
    }

    void hourBegin(unsigned int hourInTheYear)
    {
        // Next variable
        NextType::hourBegin(hourInTheYear);
    }

    void hourForEachArea(State& state, unsigned int numSpace)
    {
        unsigned int hourInYear = state.hourInTheYear;
        for (uint clusterIndex = 0; clusterIndex != state.area->shortTermStorage.count();
             ++clusterIndex)
        {
            const auto& stsHourlyResults = state.hourlyResults->ShortTermStorage[state.hourInTheWeek];
            // ST storage injection for the current cluster and this hour
            // CashFlow[h] = (withdrawal - injection) * MRG. PRICE
            pValuesForTheCurrentYear[numSpace][clusterIndex].hour[hourInYear]
                = (stsHourlyResults.withdrawal[clusterIndex]
                 - stsHourlyResults.injection[clusterIndex])
                * (-state.hourlyResults->CoutsMarginauxHoraires[state.hourInTheWeek]);
            // Note: The marginal price provided by the solver is negative (naming convention).
        }

        // Next variable
        NextType::hourForEachArea(state, numSpace);
    }

    inline void buildDigest(SurveyResults& results, int digestLevel, int dataLevel) const
    {
        // Ask to build the digest to the next variable
        NextType::buildDigest(results, digestLevel, dataLevel);
    }

    Antares::Memory::Stored<double>::ConstReturnType retrieveRawHourlyValuesForCurrentYear(
      unsigned int column,
      unsigned int numSpace) const
    {
        return pValuesForTheCurrentYear[numSpace][column].hour;
    }

    inline uint64_t memoryUsage() const
    {
        uint64_t r = (sizeof(IntermediateValues) * nbClusters_ + IntermediateValues::MemoryUsage())
                         * pNbYearsParallel;
        r += sizeof(double) * nbClusters_ * maxHoursInAYear * pNbYearsParallel;
        r += AncestorType::memoryUsage();
        return r;
    }

    void localBuildAnnualSurveyReport(SurveyResults& results,
                                      int fileLevel,
                                      int precision,
                                      unsigned int numSpace) const
    {
        // Initializing external pointer on current variable non applicable status
        results.isCurrentVarNA = AncestorType::isNonApplicable;

        if (AncestorType::isPrinted[0])
        {
            assert(NULL != results.data.area);
            const auto& shortTermStorage = results.data.area->shortTermStorage;

            // Write the data for the current year
            for (uint clusterIndex = 0; clusterIndex < nbClusters_; ++clusterIndex)
            {
                // Write the data for the current year
                const auto* cluster = shortTermStorage.storagesByIndex[clusterIndex];
                results.variableCaption = cluster->properties.name;
                results.variableUnit = VCardType::Unit();
                pValuesForTheCurrentYear[numSpace][clusterIndex]
                  .template buildAnnualSurveyReport<VCardType>(results, fileLevel, precision);
            }
        }
    }

private:
    //! Intermediate values for each year
    typename VCardType::IntermediateValuesType pValuesForTheCurrentYear = nullptr;
    size_t nbClusters_ = 0;
    unsigned int pNbYearsParallel = 0;

}; // class STstorageCashFlowByCluster

} // End namespace Antares::Solver::Variable::Economy
