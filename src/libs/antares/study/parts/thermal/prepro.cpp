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
#include <yuni/io/file.h>
#include <yuni/io/directory.h>
#include <yuni/core/math.h>
#include "../../study.h"
#include "prepro.h"
#include <antares/logs/logs.h>

using namespace Yuni;

#define SEP IO::Separator

namespace Antares
{
namespace Data
{
PreproThermal::PreproThermal(std::weak_ptr<const ThermalCluster> cluster) :
 itsThermalCluster(cluster)
{
}

void PreproThermal::copyFrom(const PreproThermal& rhs)
{
    itsThermalCluster = rhs.itsThermalCluster;
    data = rhs.data;
    rhs.data.unloadFromMemory();
}

bool PreproThermal::saveToFolder(const AnyString& folder)
{
    if (IO::Directory::Create(folder))
    {
        String buffer;
        buffer.clear() << folder << SEP << "data.txt";
        return data.saveToCSVFile(buffer, /*decimal*/ 6);
    }
    return false;
}

bool PreproThermal::loadFromFolder(Study& study, const AnyString& folder)
{
    bool ret = true;
    auto& buffer = study.bufferLoadingTS;

    auto cluster = itsThermalCluster.lock();
    if (!cluster)
        return false;

    auto parentArea = cluster->parentArea;

    buffer.clear() << folder << SEP << "data.txt";

    // standard loading
    ret = data.loadFromCSVFile(
            buffer, thermalPreproMax, DAYS_PER_YEAR, Matrix<>::optFixedSize, &study.dataBuffer)
          and ret;

    bool thermalTSglobalGeneration = study.parameters.isTSGeneratedByPrepro(timeSeriesThermal);
    if (study.usedByTheSolver && cluster->doWeGenerateTS(thermalTSglobalGeneration))
    {
        auto& colFoRate = data[foRate];
        auto& colPoRate = data[poRate];
        auto& colFoDuration = data[foDuration];
        auto& colPoDuration = data[poDuration];
        auto& colNPOMin = data[npoMin];
        auto& colNPOMax = data[npoMax];
        uint errors = 0;

        for (uint i = 0; i != DAYS_PER_YEAR; ++i)
        {
            double foRate = colFoRate[i];
            double poRate = colPoRate[i];
            double foDuration = colFoDuration[i];
            double poDuration = colPoDuration[i];
            double cNPOMin = colNPOMin[i];
            double cNPOMax = colNPOMax[i];

            if (cNPOMin < 0)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": NPO min can not be negative (line:" << (i + 1) << ")";
                ++errors;
            }
            if (cNPOMax < 0)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": NPO max can not be negative (line:" << (i + 1) << ")";
                ++errors;
            }
            if (cNPOMin > cNPOMax)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": NPO max must be greater or equal to NPO min (line:" << (i + 1)
                             << ")";
                ++errors;
            }

            if (foRate < 0. or foRate > 1.)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": invalid value for FO rate (line:" << (i + 1) << ")";
                ++errors;
            }

            if (poRate < 0. or poRate > 1.)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": invalid value for PO rate (line:" << (i + 1) << ")";
                ++errors;
            }

            if (foDuration < 1. or foDuration > 365.)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": invalid value for FO Duration (line:" << (i + 1) << ")";
                ++errors;
            }
            if (poDuration < 1. or poDuration > 365.)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": invalid value for PO Duration (line:" << (i + 1) << ")";
                ++errors;
            }

            if (errors > 30)
            {
                logs.error() << "Thermal: Prepro: " << parentArea->id << '/' << cluster->id()
                             << ": too many errors. skipping";
                break;
            }
        }
    }

    return ret;
}

bool PreproThermal::forceReload(bool reload) const
{
    return data.forceReload(reload); 
}

void PreproThermal::markAsModified() const
{
    data.markAsModified();
}

void PreproThermal::reset()
{
    data.reset(thermalPreproMax, DAYS_PER_YEAR, true);

    auto& colFoDuration = data[foDuration];
    auto& colPoDuration = data[poDuration];

    for (uint i = 0; i != DAYS_PER_YEAR; ++i)
    {
        colFoDuration[i] = 1.;
        colPoDuration[i] = 1.;
    }
}

bool PreproThermal::normalizeAndCheckNPO()
{
    auto cluster = itsThermalCluster.lock();
    if (!cluster)
        return false;

    auto parentArea = cluster->parentArea;

    // alias to our data columns
    auto& columnNPOMax = data[npoMax];
    auto& columnNPOMin = data[npoMin];
    // errors management
    uint errors = 0;
    enum
    {
        maxErrors = 10
    };
    // Flag to determine whether the column NPO max has been normalized or not
    bool normalized = false;

    for (uint y = 0; y != data.height; ++y)
    {
        if (columnNPOMax[y] > cluster->unitCount)
        {
            columnNPOMax[y] = cluster->unitCount;
            normalized = true;
        }

        if (columnNPOMin[y] > columnNPOMax[y])
        {
            if (++errors < maxErrors)
            {
                logs.error() << cluster->id() << " in area " << cluster->parentArea->id
                             << ": NPO min can not be greater than NPO max (hour: " << (y + 1)
                             << ", npo-min: " << columnNPOMin[y] << ", npo-max: " << columnNPOMax[y]
                             << ')';
            }
        }
    }

    if (errors >= maxErrors)
        logs.error() << cluster->id() << " in area " << parentArea->id
                     << ": too many errors. skipping (total: " << errors << ')';

    if (normalized)
        logs.info() << "  NPO max for the thermal cluster '" << parentArea->id
                    << "' has been normalized";

    data.markAsModified();
    return (0 == errors);
}

} // namespace Data
} // namespace Antares
