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
#ifndef __ANTARES_TOOLBOX_SPOTLIGHT_AREA_H__
#define __ANTARES_TOOLBOX_SPOTLIGHT_AREA_H__

#include <yuni/yuni.h>
#include <ui/common/component/spotlight.h>
#include <antares/study/study.h>

#include <memory>

namespace Antares
{
namespace Toolbox
{
namespace Spotlight
{
class ItemArea : public Component::Spotlight::IItem
{
public:
    using Ptr = std::shared_ptr<ItemArea>;

    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    explicit ItemArea(Data::Area* a);
    //! Destructor
    virtual ~ItemArea();
    //@}

public:
    Data::Area* area;
};

} // namespace Spotlight
} // namespace Toolbox
} // namespace Antares

#endif // __ANTARES_TOOLBOX_SPOTLIGHT_AREA_H__
