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
#ifndef __ANTARES_LIBS_STUDY_PARTS_THERMAL_CLUSTER_H__
#define __ANTARES_LIBS_STUDY_PARTS_THERMAL_CLUSTER_H__

#include <yuni/yuni.h>
#include <yuni/core/noncopyable.h>
#include <antares/array/matrix.h>
#include "defines.h"
#include "prepro.h"
#include "ecoInput.h"
#include "../common/cluster.h"
#include "../../fwd.h"
#include "pollutant.h"
#include <set>
#include <map>
#include <memory>
#include <vector>

namespace Antares
{
namespace Data
{
enum ThermalLaw
{
    thermalLawUniform,
    thermalLawGeometric
};

enum ThermalModulation
{
    thermalModulationCost = 0,
    thermalModulationMarketBid,
    thermalModulationCapacity,
    thermalMinGenModulation,
    thermalModulationMax
};

enum CostGeneration
{
    setManually = 0,
    useCostTimeseries
};

enum class LocalTSGenerationBehavior
{
    useGlobalParameter = 0,
    forceGen,
    forceNoGen
};

/*!
** \brief A single thermal cluster
*/
class ThermalCluster final : public Cluster, public std::enable_shared_from_this<ThermalCluster>
{
public:
    enum ThermalDispatchableGroup
    {
        //! Nuclear
        thermalDispatchGrpNuclear = 0,
        //! Lignite
        thermalDispatchGrpLignite,
        //! Hard Coal
        thermalDispatchGrpHardCoal,
        //! Gas
        thermalDispatchGrpGas,
        //! Oil
        thermalDispatchGrpOil,
        //! Mixed fuel
        thermalDispatchGrpMixedFuel,
        //! Other 1
        thermalDispatchGrpOther1,
        //! Other 2
        thermalDispatchGrpOther2,
        //! Other 3
        thermalDispatchGrpOther3,
        //! Other 4
        thermalDispatchGrpOther4,

        //! The highest value
        groupMax
    };

    Pollutant emissions;

    //! Set of thermal clusters
    using Set = std::set<ThermalCluster*, CompareClusterName>;
    //! Set of thermal clusters (pointer)
    using SetPointer = std::set<ThermalCluster*>;
    //! Vector of thermal clusters
    using Vector = std::vector<Data::ThermalCluster*>;

    /*!
    ** \brief Get the group name string
    ** \return A valid CString
    */
    static const char* GroupName(enum ThermalDispatchableGroup grp);

    explicit ThermalCluster(Data::Area* parent);

    ThermalCluster() = delete;
    ~ThermalCluster();

    /*!
    ** \brief Invalidate all data associated to the thermal cluster
    */
    bool forceReload(bool reload) const override;

    /*!
    ** \brief Mark the thermal cluster as modified
    */
    void markAsModified() const override;

    /*!
    ** \brief Reset to default values
    **
    ** This method should only be called from the GUI
    */
    void reset() override;

    //! Set the group
    void setGroup(Data::ClusterName newgrp) override;
    //@}

    //! \name Spinning
    //@{
    /*!
    ** \brief Calculation of spinning
    **
    ** The formula is : TS[i,j] = TS[i,j] * (1 - Spinning / 100)
    */
    void calculationOfSpinning();

    //! \name MarketBid and Marginal Costs
    //@{
    /*!
    ** \brief Calculation of market bid and marginals costs per hour
    */
    void ComputeCostTimeSeries();

    /*!
    ** \brief Calculation of spinning (reverse)
    **
    ** The original formula for the calculation of the spinning is :
    ** TS[i,j] = TS[i,j] * (1 - Spinning / 100)
    **
    ** This method is used to removed the spinning, before exporting the TS matrices
    ** into the input.
    */
    void reverseCalculationOfSpinning();
    //@}

    /*!
    ** \brief Check and fix all values of a thermal cluster
    **
    ** \return False if an error has been detected and fixed with a default value
    */
    bool integrityCheck() override;

    /*!
    ** \brief Copy data from another cluster
    **
    ** The name and the id will remain untouched.
    */
    void copyFrom(const ThermalCluster& cluster);

    /*!
    ** \brief Group ID as an uint
    */
    uint groupId() const override;

    /*!
    ** \brief Get the memory consummed by the thermal cluster (in bytes)
    */
    uint64_t memoryUsage() const override;
    //@}

    //! \name validity of Min Stable Power
    //@{
    // bool minStablePowerValidity() const;

    /*!
    ** \brief Calculte the minimum modulation/ceil(modulation) from 8760 capacity modulation
    */
    void calculatMinDivModulation();

    /*!
    ** \brief Check the validity of Min Stable Power
    */
    bool checkMinStablePower();

    /*!
    ** \brief Check the validity of Min Stable Power with a new modulation value
    */
    bool checkMinStablePowerWithNewModulation(uint index, double value);
    //@}

    bool doWeGenerateTS(bool globalTSgeneration) const;

    double getOperatingCost(uint tsIndex, uint hourInTheYear) const;
    double getMarginalCost(uint tsIndex, uint hourInTheYear) const;
    double getMarketBidCost(uint hourInTheYear, uint year) const;

    // Check & correct availability timeseries for thermal availability
    // Only applies if time-series are ready-made
    void checkAndCorrectAvailability();

    bool isActive() const;

    /*!
    ** \brief The group ID
    **
    ** This value is computed from the field 'group' in 'group()
    ** \see group()
    */
    ThermalDispatchableGroup groupID;

    //! Mustrun
    bool mustrun;
    //! Mustrun (as it were at the loading of the data)
    //
    // This value might differ from mustrun, because `mustrun` might be
    // modified for different reasons.
    // Only used by the solver in adequacy mode
    bool mustrunOrigin;

    //! Nominal capacity - spinning (solver only)
    double nominalCapacityWithSpinning;

    //! \name PMin
    //@{
    //! Min. Stable Power (MW)
    double minStablePower;

    struct DivModulation
    {
        DivModulation() : value(0.0), isCalculated(false), isValidated(false)
        {
        }

        double value;
        double border;
        uint index;
        bool isCalculated;
        bool isValidated;
    } minDivModulation;

    //! Min. Up time (1..168)
    uint minUpTime;
    //! Min. Down time (1..168)
    uint minDownTime;
    //! Max entre . minUp/minDown time (1..168)
    uint minUpDownTime;
    //@}

    //! Spinning (%)
    double spinning;

    //! Efficiency (%)
    double fuelEfficiency = 100;

    //! Forced Volatility
    double forcedVolatility;
    //! Planned volatility
    double plannedVolatility;

    //! Law (ts-generator)
    ThermalLaw forcedLaw;
    //! Law (ts-generator)
    ThermalLaw plannedLaw;

    //! \name Costs
    //  Marginal (€/MWh)     MA
    //  Spread (€/MWh)       SP
    //  Fixed (€ / hour)     FI
    //  Start-up (€/start)   SU
    //  Market bid (€/ MWh)  MB
    //
    //  v3.4:
    //  O(h) = MB * P(h)
    //
    //  v3.5:
    //  solver input : MB
    //  output :
    //  O(h) = MA * P(h)
    //  if (P(h) > 0)
    //      O(h) += FI
    //  if (N(h) > N(h-1))
    //      O(h) += SU* (N(h)-N(h-1))
    //
    // \see 101206-antares-couts.doc
    //
    //  v5.0:
    // Abs( SU ) = 0 or in [0.005;5000000]
    //  v4.5:
    // Abs( MA ) = 0 or in [0.005;50000]
    // Abs( FI ) = 0 or in [0.005;50000]
    // Abs( SU ) = 0 or in [0.005;50000]
    // Abs( MB ) = 0 or in [0.005;50000]
    // SP >=0 or in [0.005;50000]
    //
    //@{

    //! Cost generation
    CostGeneration costgeneration = setManually;
    //! Marginal cost (euros/MWh)
    double marginalCost = 0;
    //! Spread (euros/MWh)
    double spreadCost = 0;
    //! Fixed cost (euros/hour)
    double fixedCost = 0;
    //! Startup cost (euros/startup)
    double startupCost = 0;
    //! Market bid cost (euros/MWh)
    double marketBidCost = 0;
    //! Variable O&M cost (euros/MWh)
    double variableomcost = 0;
    //@}

    /*!
    ** \brief thermalMinGenModulation vector used in solver only to store the year values
    ** 8760 vector
    ** PthetaInf[hour]
    */
    std::vector<double> PthetaInf;

    //! Data for the preprocessor
    PreproThermal* prepro = nullptr;

    /*!
    ** \brief Production Cost, Market Bid Cost and Marginal Cost Matrixes - Per Hour and per Time
    *Series
    */
    struct CostsTimeSeries
    {
        std::array<double, HOURS_PER_YEAR> productionCostTs;
        std::array<double, HOURS_PER_YEAR> marketBidCostTS;
        std::array<double, HOURS_PER_YEAR> marginalCostTS;
    };
    std::vector<CostsTimeSeries> costsTimeSeries;

    EconomicInputData ecoInput;

    LocalTSGenerationBehavior tsGenBehavior = LocalTSGenerationBehavior::useGlobalParameter;

    friend class ThermalClusterList;

    double computeMarketBidCost(double fuelCost, double co2EmissionFactor, double co2cost);

    unsigned int precision() const override;

private:
    // Calculation of marketBid and marginal costs hourly time series
    //
    // Calculation of market bid and marginals costs per hour
    //
    // These time series can be set 
    // Market bid and marginal costs are set manually.
    // Or if time series are used the formula is:
    // Marginal_Cost[€/MWh] = Market_Bid_Cost[€/MWh] = (Fuel_Cost[€/GJ] * 3.6 * 100 / Efficiency[%])
    // CO2_emission_factor[tons/MWh] * C02_cost[€/tons] + Variable_O&M_cost[€/MWh]
    
    void fillMarketBidCostTS();
    void fillMarginalCostTS();
    void resizeCostTS();
    void ComputeMarketBidTS();
    void MarginalCostEqualsMarketBid();
    void ComputeProductionCostTS();



}; // class ThermalCluster
} // namespace Data
} // namespace Antares

#include "cluster.hxx"

#endif /* __ANTARES_LIBS_STUDY_PARTS_THERMAL_CLUSTER_H__ */
