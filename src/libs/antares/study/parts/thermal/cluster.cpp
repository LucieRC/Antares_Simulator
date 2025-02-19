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

#include <numeric>
#include <algorithm>

#include <yuni/yuni.h>
#include <yuni/io/file.h>
#include <yuni/core/math.h>
#include <cassert>
#include <boost/algorithm/string/case_conv.hpp>
#include "../../study.h"
#include "cluster.h"
#include <antares/inifile/inifile.h>
#include <antares/logs/logs.h>
#include <antares/utils/utils.h>

using namespace Yuni;
using namespace Antares;

#define THERMALAGGREGATELIST_INITIAL_CAPACITY 10

#define SEP IO::Separator

namespace Yuni
{
namespace Extension
{
namespace CString
{
bool Into<Antares::Data::ThermalLaw>::Perform(AnyString string, TargetType& out)
{
    string.trim();
    if (string.empty())
        return false;

    if (string.equalsInsensitive("uniform"))
    {
        out = Antares::Data::thermalLawUniform;
        return true;
    }
    if (string.equalsInsensitive("geometric"))
    {
        out = Antares::Data::thermalLawGeometric;
        return true;
    }
    return false;
}

bool Into<Antares::Data::CostGeneration>::Perform(AnyString string, TargetType& out)
{
    string.trim();
    if (string.empty())
        return false;

    if (string.equalsInsensitive("setManually"))
    {
        out = Antares::Data::setManually;
        return true;
    }
    if (string.equalsInsensitive("useCostTimeseries"))
    {
        out = Antares::Data::useCostTimeseries;
        return true;
    }
    return false;
}

bool Into<Antares::Data::LocalTSGenerationBehavior>::Perform(AnyString string, TargetType& out)
{
    string.trim();
    if (string.empty())
        return false;

    if (string.equalsInsensitive("use global"))
    {
        out = Antares::Data::LocalTSGenerationBehavior::useGlobalParameter;
        return true;
    }
    if (string.equalsInsensitive("force generation"))
    {
        out = Antares::Data::LocalTSGenerationBehavior::forceGen;
        return true;
    }
    if (string.equalsInsensitive("force no generation"))
    {
        out = Antares::Data::LocalTSGenerationBehavior::forceNoGen;
        return true;
    }
    return false;
}

} // namespace CString
} // namespace Extension
} // namespace Yuni

namespace Antares
{
namespace Data
{
Data::ThermalCluster::ThermalCluster(Area* parent) :
    Cluster(parent),
    groupID(thermalDispatchGrpOther1),
    mustrun(false),
    mustrunOrigin(false),
    nominalCapacityWithSpinning(0.),
    minStablePower(0.),
    minUpTime(1),
    minDownTime(1),
    spinning(0.),
    forcedVolatility(0.),
    plannedVolatility(0.),
    forcedLaw(thermalLawUniform),
    plannedLaw(thermalLawUniform),
    PthetaInf(HOURS_PER_YEAR, 0),
    costsTimeSeries(1, CostsTimeSeries())
{
    // assert
    assert(parent && "A parent for a thermal dispatchable cluster can not be null");
}

Data::ThermalCluster::~ThermalCluster()
{
    delete prepro;
}

uint ThermalCluster::groupId() const
{
    return groupID;
}

void Data::ThermalCluster::copyFrom(const ThermalCluster& cluster)
{
    // Note: In this method, only the data can be copied (and not the name or
    //   the ID for example)

    // mustrun
    mustrun = cluster.mustrun;
    mustrunOrigin = cluster.mustrunOrigin;

    // group
    groupID = cluster.groupID;
    pGroup = cluster.pGroup;

    // Enabled
    enabled = cluster.enabled;

    // unit count
    unitCount = cluster.unitCount;
    // nominal capacity
    nominalCapacity = cluster.nominalCapacity;
    nominalCapacityWithSpinning = cluster.nominalCapacityWithSpinning;

    minDivModulation = cluster.minDivModulation;

    minStablePower = cluster.minStablePower;
    minUpTime = cluster.minUpTime;
    minDownTime = cluster.minDownTime;

    // spinning
    spinning = cluster.spinning;

    // emissions
    emissions = cluster.emissions;

    // efficiency
    fuelEfficiency = cluster.fuelEfficiency;

    // volatility
    forcedVolatility = cluster.forcedVolatility;
    plannedVolatility = cluster.plannedVolatility;
    // law
    forcedLaw = cluster.forcedLaw;
    plannedLaw = cluster.plannedLaw;

    // costs
    costgeneration = cluster.costgeneration;
    marginalCost = cluster.marginalCost;
    spreadCost = cluster.spreadCost;
    variableomcost = cluster.variableomcost;
    fixedCost = cluster.fixedCost;
    startupCost = cluster.startupCost;
    marketBidCost = cluster.marketBidCost;
    // assignment “=” operator can be used to copy-paste vector by value
    costsTimeSeries = cluster.costsTimeSeries;

    // modulation
    modulation = cluster.modulation;
    cluster.modulation.unloadFromMemory();

    // Making sure that the data related to the prepro and timeseries are present
    // prepro
    if (not prepro)
        prepro = new PreproThermal(this->weak_from_this());

    prepro->copyFrom(*cluster.prepro);
    ecoInput.copyFrom(cluster.ecoInput);
    // timseries

    series.timeSeries = cluster.series.timeSeries;
    cluster.series.timeSeries.unloadFromMemory();
    series.timeseriesNumbers.clear();

    // The parent must be invalidated to make sure that the clusters are really
    // re-written at the next 'Save' from the user interface.
    if (parentArea)
        parentArea->forceReload();
}

static Data::ThermalCluster::ThermalDispatchableGroup stringToGroup(Data::ClusterName& newgrp)
{
    using namespace Antares::Data;
    const static std::map<ClusterName, ThermalCluster::ThermalDispatchableGroup> mapping
      = {{"nuclear", ThermalCluster::thermalDispatchGrpNuclear},
         {"lignite", ThermalCluster::thermalDispatchGrpLignite},
         {"hard coal", ThermalCluster::thermalDispatchGrpHardCoal},
         {"gas", ThermalCluster::thermalDispatchGrpGas},
         {"oil", ThermalCluster::thermalDispatchGrpOil},
         {"mixed fuel", ThermalCluster::thermalDispatchGrpMixedFuel},
         {"other", ThermalCluster::thermalDispatchGrpOther1},
         {"other 1", ThermalCluster::thermalDispatchGrpOther1},
         {"other 2", ThermalCluster::thermalDispatchGrpOther2},
         {"other 3", ThermalCluster::thermalDispatchGrpOther3},
         {"other 4", ThermalCluster::thermalDispatchGrpOther4}};

    boost::to_lower(newgrp);
    if (auto res = mapping.find(newgrp);res != mapping.end())
    {
        return res->second;
    }
    // assigning a default value
    return ThermalCluster::thermalDispatchGrpOther1;
}

void Data::ThermalCluster::setGroup(Data::ClusterName newgrp)
{
    if (newgrp.empty())
    {
        groupID = thermalDispatchGrpOther1;
        pGroup.clear();
        return;
    }
    pGroup = newgrp;
    groupID = stringToGroup(newgrp);
}

bool Data::ThermalCluster::forceReload(bool reload) const
{
    bool ret = true;
    ret = modulation.forceReload(reload) && ret;
    ret = series.forceReload(reload) && ret;
    if (prepro)
        ret = prepro->forceReload(reload) && ret;
    ret = ecoInput.forceReload(reload) && ret;
    return ret;
}

void Data::ThermalCluster::markAsModified() const
{
    modulation.markAsModified();
    series.markAsModified();
    if (prepro)
        prepro->markAsModified();
    ecoInput.markAsModified();
}

void Data::ThermalCluster::calculationOfSpinning()
{
    // nominal capacity (for solver)
    nominalCapacityWithSpinning = nominalCapacity;

    // Nothing to do if the spinning is equal to zero
    // because it will the same multiply all entries of the matrix by 1.
    if (not Math::Zero(spinning))
    {
        logs.debug() << "  Calculation of spinning... " << parentArea->name << "::" << pName;

        auto& ts = series.timeSeries;
        // The formula
        // const double s = 1. - cluster.spinning / 100.; */

        // All values in the matrix will be multiply by this coeff
        // It is no really useful to test if the result of the formula
        // is equal to zero, since the method `Matrix::multiplyAllValuesBy()`
        // already does this test.
        nominalCapacityWithSpinning *= 1 - (spinning / 100.);
        ts.multiplyAllEntriesBy(1. - (spinning / 100.));
    }
}

void Data::ThermalCluster::ComputeCostTimeSeries()
{
    switch (costgeneration)
    {
    case Data::setManually:
        fillMarketBidCostTS();
        fillMarginalCostTS();
        break;
    case Data::useCostTimeseries:
        resizeCostTS();
        ComputeMarketBidTS();
        MarginalCostEqualsMarketBid();
        ComputeProductionCostTS();
        break;
    }
}

void Data::ThermalCluster::fillMarketBidCostTS()
{
    std::fill(costsTimeSeries[0].marketBidCostTS.begin(),
              costsTimeSeries[0].marketBidCostTS.end(),
              marketBidCost);
}

void Data::ThermalCluster::fillMarginalCostTS()
{
    std::fill(costsTimeSeries[0].marginalCostTS.begin(),
              costsTimeSeries[0].marginalCostTS.end(),
              marginalCost);
}

void ThermalCluster::resizeCostTS()
{
    const uint fuelCostWidth = ecoInput.fuelcost.width;
    const uint co2CostWidth = ecoInput.co2cost.width;
    const uint tsCount = std::max(fuelCostWidth, co2CostWidth);

    costsTimeSeries.resize(tsCount, CostsTimeSeries());
}

void ThermalCluster::ComputeMarketBidTS()
{
    const uint fuelCostWidth = ecoInput.fuelcost.width;
    const uint co2CostWidth = ecoInput.co2cost.width;

    double& co2EmissionFactor = emissions.factors[Pollutant::CO2];

    for (uint tsIndex = 0; tsIndex < costsTimeSeries.size(); ++tsIndex)
    {
        uint tsIndexFuel = std::min(fuelCostWidth - 1, tsIndex);
        uint tsIndexCo2 = std::min(co2CostWidth - 1, tsIndex);
        for (uint hour = 0; hour < HOURS_PER_YEAR; ++hour)
        {
            double& marketBidCostTS = costsTimeSeries[tsIndex].marketBidCostTS[hour];

            double& fuelcost = ecoInput.fuelcost[tsIndexFuel][hour];
            double& co2cost = ecoInput.co2cost[tsIndexCo2][hour];

            marketBidCostTS = computeMarketBidCost(fuelcost, co2EmissionFactor, co2cost);
        }
    }
}

void ThermalCluster::MarginalCostEqualsMarketBid()
{
    for (auto& timeSeries : costsTimeSeries)
    {
        auto& source = timeSeries.marketBidCostTS;
        auto& destination = timeSeries.marginalCostTS;
        std::copy(source.begin(), source.end(), destination.begin());
    }
}

void ThermalCluster::ComputeProductionCostTS()
{
    if (modulation.width == 0)
        return;

    for (auto& timeSeries : costsTimeSeries)
    {
        auto& productionCostTS = timeSeries.productionCostTs;
        auto& marginalCostTS = timeSeries.marginalCostTS;

        for (uint hour = 0; hour < HOURS_PER_YEAR; ++hour)
        {
            double& houtlyModulation = modulation[Data::thermalModulationCost][hour];
            productionCostTS[hour] = marginalCostTS[hour] * houtlyModulation;
        }
    }
}


double Data::ThermalCluster::computeMarketBidCost(double fuelCost,
                                                         double co2EmissionFactor,
                                                         double co2cost)
{
    return fuelCost * 360.0 / fuelEfficiency + co2EmissionFactor * co2cost + variableomcost;
}

void Data::ThermalCluster::reverseCalculationOfSpinning()
{
    // Nothing to do if the spinning is equal to zero
    // because it will the same multiply all entries of the matrix by 1.
    if (not Math::Zero(spinning))
    {
        logs.debug() << "  Calculation of spinning (reverse)... " << parentArea->name
                     << "::" << pName;

        auto& ts = series.timeSeries;
        // The formula
        // const double s = 1. - cluster.spinning / 100.;

        // All values in the matrix will be multiply by this coeff
        // It is no really useful to test if the result of the formula
        // is equal to zero, since the method `Matrix::multiplyAllValuesBy()`
        // already does this test.
        ts.multiplyAllEntriesBy(1. / (1. - (spinning / 100.)));
        ts.roundAllEntries();
    }
}

void Data::ThermalCluster::reset()
{
    Cluster::reset();

    mustrun = false;
    mustrunOrigin = false;
    nominalCapacityWithSpinning = 0.;
    minDivModulation.isCalculated = false;
    minStablePower = 0.;
    minUpDownTime = 1;
    minUpTime = 1;
    minDownTime = 1;

    // spinning
    spinning = 0.;

    // efficiency
    fuelEfficiency = 100.0;

    // pollutant emissions array
    emissions.factors.fill(0);
    // volatility
    forcedVolatility = 0.;
    plannedVolatility = 0.;
    // laws
    plannedLaw = thermalLawUniform;
    forcedLaw = thermalLawUniform;

    // costs
    costgeneration = setManually;
    marginalCost = 0.;
    spreadCost = 0.;
    fixedCost = 0.;
    startupCost = 0.;
    marketBidCost = 0.;
    variableomcost = 0.;
    costsTimeSeries.resize(1, CostsTimeSeries());

    // modulation
    modulation.resize(thermalModulationMax, HOURS_PER_YEAR);
    modulation.fill(1.);
    modulation.fillColumn(thermalMinGenModulation, 0.);

    // prepro
    // warning: the variables `prepro` and `series` __must__ not be destroyed
    //   since the interface may still have a pointer to them.
    //   we must simply reset their content.
    if (not prepro)
        prepro = new PreproThermal(this->weak_from_this());
    prepro->reset();
    ecoInput.reset();
}

bool Data::ThermalCluster::integrityCheck()
{
    if (not parentArea)
    {
        logs.error() << "Thermal cluster " << pName << ": The parent area is missing";
        return false;
    }

    if (Math::NaN(marketBidCost))
    {
        logs.error() << "Thermal cluster " << pName << ": NaN detected for market bid cost";
        return false;
    }
    if (Math::NaN(marginalCost))
    {
        logs.error() << "Thermal cluster " << parentArea->name << '/' << pName
                     << ": NaN detected for marginal cost";
        return false;
    }
    if (Math::NaN(spreadCost))
    {
        logs.error() << "Thermal cluster " << parentArea->name << '/' << pName
                     << ": NaN detected for marginal cost";
        return false;
    }

    bool ret = true;

    if (minUpTime > 168 or 0 == minUpTime)
    {
        logs.error() << "Thermal cluster " << parentArea->name << "/" << pName
                     << ": The min. up time must be between 1 and 168";
        minUpTime = 1;
        ret = false;
    }
    if (minDownTime > 168 or 0 == minDownTime)
    {
        logs.error() << "Thermal cluster " << parentArea->name << "/" << pName
                     << ": The min. down time must be between 1 and 168";
        minDownTime = 1;
        ret = false;
    }
    if (nominalCapacity < 0.)
    {
        logs.error() << "Thermal cluster " << parentArea->name << "/" << pName
                     << ": The Nominal capacity must be positive or null";
        nominalCapacity = 0.;
        nominalCapacityWithSpinning = 0.;
        ret = false;
    }
    if (spinning < 0. or spinning > 100.)
    {
        if (spinning < 0.)
            spinning = 0;
        else
            spinning = 100.;
        logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName
                     << ": The spinning must be within the range [0,+100] (rounded to " << spinning
                     << ')';
        ret = false;
        nominalCapacityWithSpinning = nominalCapacity;
    }
    // emissions
    for (auto i = 0; i < Pollutant::POLLUTANT_MAX; i++)
    {
        if (emissions.factors[i] < 0)
        {
            logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName << ": The "
                         << Pollutant::getPollutantName(i) << " pollutant factor must be >= 0";
        }
    }
    if (fuelEfficiency <= 0. || fuelEfficiency > 100.)
    {
        fuelEfficiency = 100.;
        logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName
                     << ": The efficiency must be within the range (0,+100] (rounded to "
                     << fuelEfficiency << ')';
        ret = false;
    }
    if (spreadCost < 0.)
    {
        logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName
                     << ": The spread must be positive or null";
        spreadCost = 0.;
        ret = false;
    }
    if (variableomcost < 0.)
    {
        logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName
                     << ": The variable operation & maintenance cost must be positive or null";
        variableomcost = 0.;
        ret = false;
    }

    // Modulation
    if (modulation.height > 0)
    {
        CString<ant_k_cluster_name_max_length + ant_k_area_name_max_length + 50, false> buffer;
        buffer << "Thermal cluster: " << parentArea->name << '/' << pName << ": Modulation";
        ret = MatrixTestForPositiveValues(buffer.c_str(), &modulation) && ret;
    }

    // la valeur minStablePower should not be modified
    /*
    if (minStablePower > nominalCapacity)
    {
            logs.error() << "Thermal cluster: " << parentArea->name << '/' << pName
                    << ": failed min stable power < nominal capacity (with min power = "
                    << minStablePower << ", nominal power = " << nominalCapacity;
            minStablePower = nominalCapacity;
            ret = false;
    }*/

    return ret;
}

const char* Data::ThermalCluster::GroupName(enum ThermalDispatchableGroup grp)
{
    switch (grp)
    {
    case thermalDispatchGrpNuclear:
        return "Nuclear";
    case thermalDispatchGrpLignite:
        return "Lignite";
    case thermalDispatchGrpHardCoal:
        return "Hard Coal";
    case thermalDispatchGrpGas:
        return "Gas";
    case thermalDispatchGrpOil:
        return "Oil";
    case thermalDispatchGrpMixedFuel:
        return "Mixed Fuel";
    case thermalDispatchGrpOther1:
        return "Other";
    case thermalDispatchGrpOther2:
        return "Other 2";
    case thermalDispatchGrpOther3:
        return "Other 3";
    case thermalDispatchGrpOther4:
        return "Other 4";

    case groupMax:
        return "";
    }
    return "";
}

uint64_t ThermalCluster::memoryUsage() const
{
    uint64_t amount = sizeof(ThermalCluster) + modulation.memoryUsage();
    if (prepro)
        amount += prepro->memoryUsage();
    amount += series.memoryUsage();
    amount += ecoInput.memoryUsage();
    return amount;
}

void ThermalCluster::calculatMinDivModulation()
{
    minDivModulation.value = (modulation[thermalModulationCapacity][0]
                              / Math::Ceil(modulation[thermalModulationCapacity][0]));
    minDivModulation.index = 0;

    for (uint t = 1; t < modulation.height; t++)
    {
        double div = modulation[thermalModulationCapacity][t]
                     / ceil(modulation[thermalModulationCapacity][t]);

        if (div < minDivModulation.value)
        {
            minDivModulation.value = div;
            minDivModulation.index = t;
        }
    }
    minDivModulation.isCalculated = true;
}

bool ThermalCluster::checkMinStablePower()
{
    if (not minDivModulation.isCalculated) // not has been initialized
        calculatMinDivModulation();

    if (minDivModulation.value < 0)
    {
        minDivModulation.isValidated = false;
        return false;
    }

    // calculate nominalCapacityWithSpinning
    double nomCapacityWithSpinning = nominalCapacity * (1 - spinning / 101);

    if (Math::Zero(1 - spinning / 101))
        minDivModulation.border = .0;
    else
        minDivModulation.border
          = Math::Min(nomCapacityWithSpinning, minStablePower) / nomCapacityWithSpinning;

    if (minDivModulation.value < minDivModulation.border)
    {
        minDivModulation.isValidated = false;
        return false;
    }

    minDivModulation.isValidated = true;
    return true;
}

bool ThermalCluster::checkMinStablePowerWithNewModulation(uint index, double value)
{
    if (not minDivModulation.isCalculated || index == minDivModulation.index)
        calculatMinDivModulation();
    else
    {
        double div = value / ceil(value);
        if (div < minDivModulation.value)
        {
            minDivModulation.value = div;
            minDivModulation.index = index;
        }
    }

    return checkMinStablePower();
}

bool ThermalCluster::doWeGenerateTS(bool globalTSgeneration) const
{
    switch (tsGenBehavior)
    {
    case LocalTSGenerationBehavior::useGlobalParameter:
        return globalTSgeneration;
    case LocalTSGenerationBehavior::forceGen:
        return true;
    default:
        return false;
    }
}

unsigned int ThermalCluster::precision() const
{
    return 0;
}

double ThermalCluster::getOperatingCost(uint serieIndex, uint hourInTheYear) const
{
    if (costgeneration == Data::setManually)
    {
        const auto* modCost = modulation[thermalModulationCost];
        return marginalCost * modCost[hourInTheYear];
    }
    else
    {
        const uint tsIndex = Math::Min(serieIndex, costsTimeSeries.size() - 1);
        return costsTimeSeries[tsIndex].productionCostTs[hourInTheYear];
    }
}

double ThermalCluster::getMarginalCost(uint serieIndex, uint hourInTheYear) const
{
    const double mod = modulation[Data::thermalModulationCost][hourInTheYear];

    if (costgeneration == Data::setManually)
    {
        return marginalCost * mod;
    }
    else
    {
        const uint tsIndex = Math::Min(serieIndex, costsTimeSeries.size() - 1);
        return costsTimeSeries[tsIndex].marginalCostTS[hourInTheYear] * mod;
    }
    /* Math::Min is necessary in case Availability has e.g 10 TS and both FuelCost & Co2Cost have
     only 1TS. Then - > In order to save memory marginalCostTS vector has only one array
     inside -> that is used for all (e.g.10) TS*/
}

double ThermalCluster::getMarketBidCost(uint hourInTheYear, uint year) const
{
    uint serieIndex = series.getSeriesIndex(year);

    double mod = modulation[thermalModulationMarketBid][serieIndex];

    if (costgeneration == Data::setManually)
    {
        return marketBidCost * mod;
    }
    else
    {
        const uint tsIndex = Math::Min(serieIndex, costsTimeSeries.size() - 1);
        return costsTimeSeries[tsIndex].marketBidCostTS[hourInTheYear] * mod;
    }
}

void ThermalCluster::checkAndCorrectAvailability()
{
    const auto PmaxDUnGroupeDuPalierThermique = nominalCapacityWithSpinning;
    const auto PminDUnGroupeDuPalierThermique = (nominalCapacityWithSpinning < minStablePower)
                                                  ? nominalCapacityWithSpinning
                                                  : minStablePower;

    bool condition = false;
    bool report = false;

    for (uint y = 0; y != series.timeSeries.height; ++y)
    {
        for (uint x = 0; x != series.timeSeries.width; ++x)
        {
            auto rightpart
              = PminDUnGroupeDuPalierThermique
                * ceil(series.timeSeries.entry[x][y] / PmaxDUnGroupeDuPalierThermique);
            condition = rightpart > series.timeSeries.entry[x][y];
            if (condition)
            {
                series.timeSeries.entry[x][y] = rightpart;
                report = true;
            }
        }
    }

    if (report)
        logs.warning() << "Area : " << parentArea->name << " cluster name : " << name()
                       << " available power lifted to match Pmin and Pnom requirements";
}

bool ThermalCluster::isActive() const {
    return enabled && !mustrun;
}

} // namespace Data
} // namespace Antares
