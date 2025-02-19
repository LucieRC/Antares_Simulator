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

#include "filter.h"

using namespace Yuni;

namespace Antares
{
namespace Data
{
std::string datePrecisionIntoString(uint datePrecisionFilter)
{
    std::string to_return;
    if (datePrecisionFilter & filterHourly)
        to_return += "hourly";

    if (datePrecisionFilter & filterDaily)
    {
        if (!to_return.empty())
            to_return += ", ";
        to_return += "daily";
    }

    if (datePrecisionFilter & filterWeekly)
    {
        if (!to_return.empty())
            to_return += ", ";
        to_return += "weekly";
    }

    if (datePrecisionFilter & filterMonthly)
    {
        if (!to_return.empty())
            to_return += ", ";
        to_return += "monthly";
    }

    if (datePrecisionFilter & filterAnnual)
    {
        if (!to_return.empty())
            to_return += ", ";
        to_return += "annual";
    }

    return to_return;
}

uint stringIntoDatePrecision(const AnyString& string)
{
    if (string.empty())
        return filterNone;

    uint flag = 0;

    string.words(",; \r\n\t", [&](const AnyString& word) -> bool {
        ShortString16 s = word;
        s.toLower();
        if (s == "hourly")
        {
            flag |= filterHourly;
            return true;
        }
        if (s == "daily")
        {
            flag |= filterDaily;
            return true;
        }
        if (s == "weekly")
        {
            flag |= filterWeekly;
            return true;
        }
        if (s == "monthly")
        {
            flag |= filterMonthly;
            return true;
        }
        if (s == "annual")
        {
            flag |= filterAnnual;
            return true;
        }
        return true;
    });
    return flag;
}

uint addTimeIntervallToDatePrecisionFilter(const uint index)
{
    uint flag = 0;
    switch (index)
    {
    case 0:
        return flag |= filterHourly;
    case 1:
        return flag |= filterDaily;
    case 2:
        return flag |= filterWeekly;
    case 3:
        return flag |= filterMonthly;
    case 4:
        return flag |= filterAnnual;
    default:
        return filterNone;
    }

    return flag;
}

} // namespace Data
} // namespace Antares
