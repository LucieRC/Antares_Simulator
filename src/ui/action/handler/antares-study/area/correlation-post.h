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
#ifndef __ANTARES_LIBS_STUDY_ACTION_HANDLER_ANTARES_AREA_CORRELATION_POST_H__
#define __ANTARES_LIBS_STUDY_ACTION_HANDLER_ANTARES_AREA_CORRELATION_POST_H__

#include <yuni/yuni.h>
#include <action/action.h>

namespace Antares
{
namespace Action
{
namespace AntaresStudy
{
namespace Area
{
class CorrelationPost : public IAction
{
public:
    //! The most suitable smart ptr for the class
    using Ptr = IAction::Ptr;
    //! The threading policy
    using ThreadingPolicy = IAction::ThreadingPolicy;

public:
    //! \name Constructor & Destructor
    //@{
    /*!
    ** \brief Default constructor
    */
    explicit CorrelationPost(IAction* parent, Data::TimeSeriesType ts, const AnyString& areaname);
    //! Destructor
    virtual ~CorrelationPost();
    //@}

    virtual bool visible() const;

protected:
    virtual bool prepareWL(Context& ctx);
    virtual bool performWL(Context& ctx);

private:
    //! The attached action
    IAction::Ptr pAction;
    Data::TimeSeriesType pType;
    Data::AreaName pOriginalAreaName;

}; // class IAction

} // namespace Area
} // namespace AntaresStudy
} // namespace Action
} // namespace Antares

#include "correlation-post.hxx"

#endif // __ANTARES_LIBS_STUDY_ACTION_HANDLER_ANTARES_AREA_CORRELATION_POST_H__
