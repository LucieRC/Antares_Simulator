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

#include "../../study.h"
#include "container.h"
#include <antares/inifile/inifile.h>

using namespace Antares;
using namespace Yuni;

#define SEP IO::Separator

namespace Antares
{
namespace Data
{
PartHydro::PartHydro() :
 interDailyBreakdown(0.),
 intraDailyModulation(2.),
 intermonthlyBreakdown(0),
 reservoirManagement(false),
 followLoadModulations(true),
 useWaterValue(false),
 hardBoundsOnRuleCurves(false),
 useHeuristicTarget(true),
 reservoirCapacity(0.),
 initializeReservoirLevelDate(0),
 useLeeway(false),
 powerToLevel(false),
 leewayLowerBound(1.),
 leewayUpperBound(1.),
 pumpingEfficiency(1.),
 hydroModulable(false),
 prepro(nullptr),
 series(nullptr)
{
}

PartHydro::~PartHydro()
{
    delete prepro;
    delete series;
}

void PartHydro::reset()
{
    intraDailyModulation = 24;
    interDailyBreakdown = 1.;
    intermonthlyBreakdown = 1.;

    reservoirManagement = false;
    reservoirCapacity = 0.;

    followLoadModulations = true;
    useWaterValue = false;
    hardBoundsOnRuleCurves = false;
    useHeuristicTarget = true;
    initializeReservoirLevelDate = 0;
    useLeeway = false;
    powerToLevel = false;
    leewayLowerBound = 1.;
    leewayUpperBound = 1.;

    inflowPattern.reset(1, DAYS_PER_YEAR, true);
    inflowPattern.fillColumn(0, 1.0);
    reservoirLevel.reset(3, DAYS_PER_YEAR, true);
    reservoirLevel.fillColumn(average, 0.5);
    reservoirLevel.fillColumn(maximum, 1.);
    waterValues.reset(101, DAYS_PER_YEAR, true);
    maxPower.reset(4, DAYS_PER_YEAR, true);
    maxPower.fillColumn(genMaxE, 24.);
    maxPower.fillColumn(pumpMaxE, 24.);
    creditModulation.reset(101, 2, true);
    creditModulation.fill(1);
    // reset of the hydro allocation - however we don't have any information
    // about the current area, which should be by default 1.
    // This work is done in Area::reset()
    allocation.clear();
    // allocation.fromArea(<current area>, 1.); // Area::reset()

    if (prepro)
        prepro->reset();
    if (series)
        series->reset();
}

bool PartHydro::LoadFromFolder(Study& study, const AnyString& folder)
{
    auto& buffer = study.bufferLoadingTS;
    bool ret = true;

    // Initialize all alpha values to 0
    study.areas.each([&](Data::Area& area) {
        area.hydro.interDailyBreakdown = 1.;
        area.hydro.intraDailyModulation = 24.;
        area.hydro.intermonthlyBreakdown = 1.;
        area.hydro.reservoirManagement = false;
        area.hydro.followLoadModulations = true;
        area.hydro.useWaterValue = false;
        area.hydro.hardBoundsOnRuleCurves = false;
        area.hydro.useHeuristicTarget = true;
        area.hydro.useLeeway = false;
        area.hydro.powerToLevel = false;
        area.hydro.leewayLowerBound = 1.;
        area.hydro.leewayUpperBound = 1.;
        area.hydro.initializeReservoirLevelDate = 0;
        area.hydro.reservoirCapacity = 0.;
        area.hydro.pumpingEfficiency = 1.;

        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "maxpower_"
                       << area.id << '.' << study.inputExtension;

       // GUI part patch :
       // We need to know, when estimating the RAM required by the solver, if the current area
       // is hydro modulable. Therefore, reading the area's daily max power at this stage is
       // necessary.

        if (!study.usedByTheSolver)
        {
            bool enabledModeIsChanged = false;
            if (JIT::enabled)
            {
                JIT::enabled = false; // Allowing to read the area's daily max power
                enabledModeIsChanged = true;
            }

            ret = area.hydro.maxPower.loadFromCSVFile(
                    buffer, 4, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
                && ret;

            if (enabledModeIsChanged)
                JIT::enabled = true; // Back to the previous loading mode.
        }
        else
        {
            ret = area.hydro.maxPower.loadFromCSVFile(
                    buffer, 4, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
                && ret;
        }

        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP
                       << "creditmodulations_" << area.id << '.' << study.inputExtension;
        ret = area.hydro.creditModulation.loadFromCSVFile(
                buffer, 101, 2, Matrix<>::optFixedSize, &study.dataBuffer)
              && ret;

        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "reservoir_"
                       << area.id << '.' << study.inputExtension;
        ret = area.hydro.reservoirLevel.loadFromCSVFile(
                buffer, 3, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
              && ret;


        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP
                       << "waterValues_" << area.id << '.' << study.inputExtension;
        ret = area.hydro.waterValues.loadFromCSVFile(
                buffer, 101, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
              && ret;

        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP
                       << "inflowPattern_" << area.id << '.' << study.inputExtension;
        ret = area.hydro.inflowPattern.loadFromCSVFile(
                buffer, 1, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
              && ret;

        if (study.usedByTheSolver)
        {
            auto& col = area.hydro.inflowPattern[0];
            bool errorInflow = false;
            for (int day = 0; day < DAYS_PER_YEAR; day++)
            {
                if (col[day] < 0 && !errorInflow)
                {
                    logs.error() << area.name << ": invalid inflow value";
                    errorInflow = true;
                    ret = false;
                }
            }
            bool errorLevels = false;
            auto& colMin = area.hydro.reservoirLevel[minimum];
            auto& colAvg = area.hydro.reservoirLevel[average];
            auto& colMax = area.hydro.reservoirLevel[maximum];
            for (int day = 0; day < DAYS_PER_YEAR; day++)
            {
                if (!errorLevels
                    && (colMin[day] < 0 || colAvg[day] < 0 || colMin[day] > colMax[day]
                        || colAvg[day] > 100 || colMax[day] > 100))
                {
                    logs.error() << area.name << ": invalid reservoir level value";
                    errorLevels = true;
                    ret = false;
                }
            }
            bool errorPowers = false;
            for (int i = 0; i < 4; i++)
            {
                auto& col = area.hydro.maxPower[i];
                for (int day = 0; day < DAYS_PER_YEAR; day++)
                {
                    if (!errorPowers && (col[day] < 0 || (i % 2 /*column hours*/ && col[day] > 24)))
                    {
                        logs.error() << area.name << ": invalid power or energy value";
                        errorPowers = true;
                        ret = false;
                    }
                }
            }
            for (int i = 0; i < 101; i++)
            {
                if ((area.hydro.creditModulation[i][0] < 0)
                    || (area.hydro.creditModulation[i][1] < 0))
                {
                    logs.error() << area.name << ": invalid credit modulation value";
                    ret = false;
                }
            }
        }
        else
        {
            // Is area hydro modulable ?
            auto& max = area.hydro.maxPower[area.hydro.genMaxP];

            for (uint y = 0; y != area.hydro.maxPower.height; ++y)
            {
                if (max[y] > 0.)
                {
                    area.hydro.hydroModulable = true;
                    break;
                }
            }
        }
    });

    IniFile ini;
    if (not ini.open(buffer.clear() << folder << SEP << "hydro.ini"))
        return false;

    const char* const sectionName = "inter-daily-breakdown";

    IniFile::Section* section;
    IniFile::Property* property;

    if ((section = ini.find(sectionName)))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                Area* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<double>(area->hydro.interDailyBreakdown) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    if ((section = ini.find("intra-daily-modulation")))
    {
        if ((property = section->firstProperty))
        {
            AreaName id;

            // Browse all properties
            for (; property; property = property->next)
            {
                id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.intraDailyModulation) && ret;
                    if (area->hydro.intraDailyModulation < 1.)
                    {
                        logs.error()
                          << area->id << ": Invalid intra-daily modulation. It must be >= 1.0, Got "
                          << area->hydro.intraDailyModulation << " (truncated to 1)";
                        area->hydro.intraDailyModulation = 1.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    if ((section = ini.find("reservoir")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.reservoirManagement) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    if ((section = ini.find("reservoir capacity")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.reservoirCapacity) && ret;
                    if (area->hydro.reservoirCapacity < 1e-6)
                    {
                        logs.error() << area->id << ": Invalid reservoir capacity.";
                        area->hydro.reservoirCapacity = 0.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    // Check on reservoir capacity (has to be done after reservoir management and capacity reading,
    // not before). Some areas reservoir capacities may not be printed in hydro ini file when saving
    // the study, because they are too small (< 1e-6). We cannot have reservoir management = yes and
    // capacity = 0 because of further division by capacity. reservoir management = no and capacity
    // = 0 is possible (no use of capacity further)
    study.areas.each([&](Data::Area& area) {
        if (area.hydro.reservoirCapacity < 1e-3 && area.hydro.reservoirManagement)
        {
            logs.error() << area.name << ": reservoir capacity not defined. Impossible to manage.";
            ret = false && ret;
        }
    });

    if ((section = ini.find("inter-monthly-breakdown")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.intermonthlyBreakdown) && ret;
                    if (area->hydro.intermonthlyBreakdown < 0)
                    {
                        logs.error() << area->id << ": Invalid intermonthly breakdown";
                        area->hydro.intermonthlyBreakdown = 0.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("follow load")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.followLoadModulations) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("use water")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.useWaterValue) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("hard bounds")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.hardBoundsOnRuleCurves) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("use heuristic")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.useHeuristicTarget) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("power to level")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.powerToLevel) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("initialize reservoir date")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<int>(area->hydro.initializeReservoirLevelDate) && ret;
                    if (area->hydro.initializeReservoirLevelDate < 0)
                    {
                        logs.error() << area->id << ": Invalid initialize reservoir date";
                        area->hydro.initializeReservoirLevelDate = 0;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    // Leeways : use leeway bounds (upper and lower)
    if ((section = ini.find("use leeway")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                    ret = property->value.to<bool>(area->hydro.useLeeway) && ret;
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("leeway low")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.leewayLowerBound) && ret;
                    if (area->hydro.leewayLowerBound < 0.)
                    {
                        logs.error()
                          << area->id << ": Invalid leeway lower bound. It must be >= 0.0, Got "
                          << area->hydro.leewayLowerBound;
                        area->hydro.leewayLowerBound = 0.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }
    if ((section = ini.find("leeway up")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.leewayUpperBound) && ret;
                    if (area->hydro.leewayUpperBound < 0.)
                    {
                        logs.error()
                          << area->id << ": Invalid leeway upper bound. It must be >= 0.0, Got "
                          << area->hydro.leewayUpperBound;
                        area->hydro.leewayUpperBound = 0.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    // they are too small (< 1e-6). We cannot allow these areas to have reservoir management =
    // true.
    study.areas.each([&](Data::Area& area) {
        if (area.hydro.leewayLowerBound > area.hydro.leewayUpperBound)
            logs.error() << area.id << ": Leeway lower bound greater than leeway upper bound.";
        });

    if ((section = ini.find("pumping efficiency")))
    {
        if ((property = section->firstProperty))
        {
            // Browse all properties
            for (; property; property = property->next)
            {
                AreaName id = property->key;
                id.toLower();

                auto* area = study.areas.find(id);
                if (area)
                {
                    ret = property->value.to<double>(area->hydro.pumpingEfficiency) && ret;
                    if (area->hydro.pumpingEfficiency < 0)
                    {
                        logs.error() << area->id << ": Invalid pumping efficiency";
                        area->hydro.pumpingEfficiency = 0.;
                    }
                }
                else
                    logs.warning() << buffer << ": `" << id << "`: Unknown area";
            }
        }
    }

    study.areas.each([&](Data::Area& area) {
        if (not area.hydro.useHeuristicTarget && not area.hydro.useWaterValue)
        {
            logs.error() << area.name
                         << " : use water value = no conflicts with use heuristic target = no";
            ret = false && ret;
        }
    });

    return ret;
}

bool PartHydro::SaveToFolder(const AreaList& areas, const AnyString& folder)
{
    if (!folder)
    {
        logs.error() << "hydro: invalid empty folder";
        assert(false && "invalid empty folder");
        return false;
    }

    String buffer;
    buffer.clear() << folder << SEP << "common" << SEP << "capacity";

    // Init
    IniFile ini;
    auto* s = ini.addSection("inter-daily-breakdown");
    auto* smod = ini.addSection("intra-daily-modulation");
    auto* sIMB = ini.addSection("inter-monthly-breakdown");
    auto* sreservoir = ini.addSection("reservoir");
    auto* sreservoirCapacity = ini.addSection("reservoir capacity");
    auto* sFollowLoad = ini.addSection("follow load");
    auto* sUseWater = ini.addSection("use water");
    auto* sHardBounds = ini.addSection("hard bounds");
    auto* sInitializeReservoirDate = ini.addSection("initialize reservoir date");
    auto* sUseHeuristic = ini.addSection("use heuristic");
    auto* sUseLeeway = ini.addSection("use leeway");
    auto* sPowerToLevel = ini.addSection("power to level");
    auto* sLeewayLow = ini.addSection("leeway low");
    auto* sLeewayUp = ini.addSection("leeway up");
    auto* spumpingEfficiency = ini.addSection("pumping efficiency");

    // return status
    bool ret = true;

    // Add all alpha values for each area
    areas.each([&](const Data::Area& area) {
        s->add(area.id, area.hydro.interDailyBreakdown);
        smod->add(area.id, area.hydro.intraDailyModulation);
        sIMB->add(area.id, area.hydro.intermonthlyBreakdown);
        sInitializeReservoirDate->add(area.id, area.hydro.initializeReservoirLevelDate);
        sLeewayLow->add(area.id, area.hydro.leewayLowerBound);
        sLeewayUp->add(area.id, area.hydro.leewayUpperBound);
        spumpingEfficiency->add(area.id, area.hydro.pumpingEfficiency);
        if (area.hydro.reservoirCapacity > 1e-6)
            sreservoirCapacity->add(area.id, area.hydro.reservoirCapacity);
        if (area.hydro.reservoirManagement)
            sreservoir->add(area.id, true);
        if (!area.hydro.followLoadModulations)
            sFollowLoad->add(area.id, false);
        if (area.hydro.useWaterValue)
            sUseWater->add(area.id, true);
        if (area.hydro.hardBoundsOnRuleCurves)
            sHardBounds->add(area.id, true);
        if (!area.hydro.useHeuristicTarget)
            sUseHeuristic->add(area.id, false);
        if (area.hydro.useLeeway)
            sUseLeeway->add(area.id, true);
        if (area.hydro.powerToLevel)
            sPowerToLevel->add(area.id, true);
        // max power
        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "maxpower_"
                       << area.id << ".txt";
        ret = area.hydro.maxPower.saveToCSVFile(buffer, /*decimal*/ 2) && ret;
        // credit modulations
        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP
                       << "creditmodulations_" << area.id << ".txt";
        ret = area.hydro.creditModulation.saveToCSVFile(buffer, /*decimal*/ 2) && ret;
        // inflow pattern
        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "inflowPattern_"
                       << area.id << ".txt";
        ret = area.hydro.inflowPattern.saveToCSVFile(buffer, /*decimal*/ 3) && ret;
        // reservoir
        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "reservoir_"
                       << area.id << ".txt";
        ret = area.hydro.reservoirLevel.saveToCSVFile(buffer, /*decimal*/ 3) && ret;
        buffer.clear() << folder << SEP << "common" << SEP << "capacity" << SEP << "waterValues_"
                       << area.id << ".txt";
        ret = area.hydro.waterValues.saveToCSVFile(buffer, /*decimal*/ 2) && ret;
    });

    // Write the ini file
    buffer.clear() << folder << SEP << "hydro.ini";
    return ini.save(buffer) && ret;
}

bool PartHydro::forceReload(bool reload) const
{
    bool ret = true;
    ret = maxPower.forceReload(reload) && ret;
    ret = creditModulation.forceReload(reload) && ret;
    ret = inflowPattern.forceReload(reload) && ret;
    ret = reservoirLevel.forceReload(reload) && ret;
    ret = waterValues.forceReload(reload) && ret;

    if (series)
        ret = series->forceReload(reload) && ret;
    if (prepro)
        ret = prepro->forceReload(reload) && ret;

    return ret;
}

void PartHydro::markAsModified() const
{
    maxPower.markAsModified();
    inflowPattern.markAsModified();
    reservoirLevel.markAsModified();
    waterValues.markAsModified();
    creditModulation.markAsModified();

    if (series)
        series->markAsModified();
    if (prepro)
        prepro->markAsModified();
}

void PartHydro::copyFrom(const PartHydro& rhs)
{
    // max power
    {
        maxPower = rhs.maxPower;
        maxPower.unloadFromMemory();
        rhs.maxPower.unloadFromMemory();
    }
    // credit modulations
    {
        creditModulation = rhs.creditModulation;
        creditModulation.unloadFromMemory();
        rhs.creditModulation.unloadFromMemory();
    }
    // inflow pattern
    {
        inflowPattern = rhs.inflowPattern;
        inflowPattern.unloadFromMemory();
        rhs.inflowPattern.unloadFromMemory();
    }
    // reservoir levels
    {
        reservoirLevel = rhs.reservoirLevel;
        reservoirLevel.unloadFromMemory();
        rhs.reservoirLevel.unloadFromMemory();
    }
    // water values
    {
        waterValues = rhs.waterValues;
        waterValues.unloadFromMemory();
        rhs.waterValues.unloadFromMemory();
    }
    // values
    {
        interDailyBreakdown = rhs.interDailyBreakdown;
        intraDailyModulation = rhs.intraDailyModulation;
        intermonthlyBreakdown = rhs.intermonthlyBreakdown;
        reservoirManagement = rhs.reservoirManagement;
        reservoirCapacity = rhs.reservoirCapacity;
        followLoadModulations = rhs.followLoadModulations;
        useWaterValue = rhs.useWaterValue;
        hardBoundsOnRuleCurves = rhs.hardBoundsOnRuleCurves;
        useHeuristicTarget = rhs.useHeuristicTarget;
        initializeReservoirLevelDate = rhs.initializeReservoirLevelDate;
        useLeeway = rhs.useLeeway;
        powerToLevel = rhs.powerToLevel;
        leewayUpperBound = rhs.leewayUpperBound;
        leewayLowerBound = rhs.leewayLowerBound;
        pumpingEfficiency = rhs.pumpingEfficiency;
    }
}

void getWaterValue(const double& level /* format : in % of reservoir capacity */,
                   const Matrix<double>& waterValues,
                   const uint day,
                   double& waterValueToReturn)
{
    assert((level >= 0. && level <= 100.) && "getWaterValue function : invalid level");
    double levelUp = ceil(level);
    double levelDown = floor(level);

    if ((int)(levelUp) == (int)(levelDown))
        waterValueToReturn = waterValues[(int)(levelUp)][day];
    else
        waterValueToReturn
          = waterValues[(int)(levelUp)][day] * (level - levelDown)
            + waterValues[(int)(levelDown)][day] * (levelUp - level);
}

double getWeeklyModulation(const double& level /* format : in % of reservoir capacity */,
                           Matrix<double, double>& creditMod,
                           int modType)
{
    assert((level >= 0. && level <= 100.) && "getWeeklyModulation function : invalid level");
    double valueToReturn = 1.;
    double levelUp = ceil(level);
    double levelDown = floor(level);
    if ((int)(levelUp) == (int)(levelDown))
        valueToReturn = creditMod[(int)(levelUp)][modType];
    else
        valueToReturn = creditMod[(int)(levelUp)][modType] * (level - levelDown)
                        + creditMod[(int)(levelDown)][modType] * (levelUp - level);

    return valueToReturn;
}

} // namespace Data
} // namespace Antares
