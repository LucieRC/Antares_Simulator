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
#ifndef __ANTARES_WINDOWS_OUTPUT_JOB_H__
#define __ANTARES_WINDOWS_OUTPUT_JOB_H__

#include <yuni/yuni.h>
#include <yuni/job/job.h>
#include <antares/study/fwd.h>
#include "output.h"
#include "content.h"

namespace Antares
{
namespace Private
{
namespace OutputViewerData
{
class Job final : public Yuni::Job::IJob
{
public:
    /*!
    ** \brief Constructor
    */
    Job(Antares::Window::OutputViewer::Component& component, const AnyString& path);
    //! Destructor
    virtual ~Job();

protected:
    virtual void onExecute() override;

    //! Gather informations about year-by-year data
    void gatherInfosAboutYearByYearData(const AnyString& path);

    //! Gather informations about thermal clusters
    void gatherInfosAboutThermalClusters(const AnyString& path);

private:
    Antares::Window::OutputViewer::Component& pComponent;
    //! The path to analyze
    const YString pPath;
    //!
    Content* pContent;

}; // class Job

} // namespace OutputViewerData
} // namespace Private
} // namespace Antares

#endif // __ANTARES_WINDOWS_OUTPUT_JOB_H__
