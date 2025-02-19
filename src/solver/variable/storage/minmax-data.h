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
#ifndef __SOLVER_VARIABLE_STORAGE_MINMAX_DATA_H__
#define __SOLVER_VARIABLE_STORAGE_MINMAX_DATA_H__

#include <antares/study/study.h>
#include <antares/memory/memory.h>

namespace Antares
{
namespace Solver
{
namespace Variable
{
namespace R
{
namespace AllYears
{
class MinMaxData
{
public:
    struct Data
    {
        double value;
        uint32_t indice;
    };

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    MinMaxData();
    //! Destructor
    ~MinMaxData();

    void initialize();

    void resetInf();
    void resetSup();

    void mergeInf(uint year, const IntermediateValues& rhs);
    void mergeSup(uint year, const IntermediateValues& rhs);

public:
    Data annual;
    Data monthly[maxMonths];
    Data weekly[maxWeeksInAYear];
    Data daily[maxDaysInAYear];
    Antares::Memory::Stored<Data>::Type hourly;

}; // class MinMaxData

} // namespace AllYears
} // namespace R
} // namespace Variable
} // namespace Solver
} // namespace Antares

#endif // __SOLVER_VARIABLE_STORAGE_MINMAX_DATA_H__
