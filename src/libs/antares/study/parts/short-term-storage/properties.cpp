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

#include <antares/logs/logs.h>
#include <stdexcept>

#include "properties.h"

#define SEP Yuni::IO::Separator

namespace Antares::Data::ShortTermStorage
{
const std::map<std::string, enum Group> Properties::ST_STORAGE_PROPERTY_GROUP_ENUM
  = {{"PSP_open", Group::PSP_open},
     {"PSP_closed", Group::PSP_closed},
     {"Pondage", Group::Pondage},
     {"Battery", Group::Battery},
     {"Other1", Group::Other1},
     {"Other2", Group::Other2},
     {"Other3", Group::Other3},
     {"Other4", Group::Other4},
     {"Other5", Group::Other5}};

unsigned int groupIndex(Group group)
{
    switch (group)
    {
    case Group::PSP_open:
        return 0;
    case Group::PSP_closed:
        return 1;
    case Group::Pondage:
        return 2;
    case Group::Battery:
        return 3;
    case Group::Other1:
        return 4;
    case Group::Other2:
        return 5;
    case Group::Other3:
        return 6;
    case Group::Other4:
        return 7;
    case Group::Other5:
        return 8;
    default:
        throw std::invalid_argument("Group not recognized");
    }
}

bool Properties::loadKey(const IniFile::Property* p)
{
    auto valueForOptional = [p](std::optional<double>& opt) {
        if (double tmp; p->value.to<double>(tmp))
        {
            opt = tmp;
            return true;
        }
        return false;
    };

    if (p->key == "injectionnominalcapacity")
        return valueForOptional(this->injectionNominalCapacity);

    if (p->key == "withdrawalnominalcapacity")
        return valueForOptional(this->withdrawalNominalCapacity);

    if (p->key == "reservoircapacity")
        return valueForOptional(this->reservoirCapacity);

    if (p->key == "efficiency")
        return p->value.to<double>(this->efficiencyFactor);

    if (p->key == "name")
        return p->value.to<std::string>(this->name);

    if (p->key == "initiallevel")
        return p->value.to<double>(this->initialLevel);

    if (p->key == "initialleveloptim")
        return p->value.to<bool>(this->initialLevelOptim);

    if (p->key == "group")
    {
        if (auto it = Properties::ST_STORAGE_PROPERTY_GROUP_ENUM.find(p->value.c_str());
            it != Properties::ST_STORAGE_PROPERTY_GROUP_ENUM.end())
        {
            this->group = it->second;
            return true;
        }
        return false;
    }

    return false;
}

bool Properties::saveToFolder(const std::string& folder) const
{
    const std::string pathIni(folder + SEP + "list.ini");

    // Make sure the folder is created
    if (!Yuni::IO::Directory::Create(folder))
    {
        logs.warning() << "Couldn't create dir for sts: " << folder;
        return false;
    }

    logs.debug() << "saving file " << pathIni;

    IniFile ini;
    IniFile::Section* s = ini.addSection(this->name);

    s->add("name", this->name);

    for (const auto& [key, value] : ST_STORAGE_PROPERTY_GROUP_ENUM)
        if (value == this->group)
            s->add("group", key);

    s->add("reservoircapacity", this->reservoirCapacity);
    s->add("initiallevel", this->initialLevel);
    s->add("injectionnominalcapacity", this->injectionNominalCapacity);
    s->add("withdrawalnominalcapacity", this->withdrawalNominalCapacity);

    s->add("efficiency", this->efficiencyFactor);
    s->add("initialleveloptim", this->initialLevelOptim);


    return ini.save(pathIni);
}

bool Properties::validate()
{
    auto checkMandatory = [this](const std::optional<double>& prop, const std::string& label) {
        if (!prop.has_value())
        {
            logs.error() << "Property " << label << " is mandatory for short term storage "
                         << this->name;
            return false;
        }
        return true;
    };

    if (!checkMandatory(injectionNominalCapacity, "injectionnominalcapacity"))
        return false;

    if (!checkMandatory(withdrawalNominalCapacity, "withdrawalnominalcapacity"))
        return false;

    if (!checkMandatory(reservoirCapacity, "reservoircapacity"))
        return false;

    if (injectionNominalCapacity < 0)
    {
        logs.error() << "Property injectionnominalcapacity must be >= 0 "
                     << "for short term storage " << name;
        return false;
    }
    if (withdrawalNominalCapacity < 0)
    {
        logs.error() << "Property withdrawalnominalcapacity must be >= 0 "
                     << "for short term storage " << name;
        return false;
    }
    if (reservoirCapacity < 0)
    {
        logs.error() << "Property reservoircapacity must be >= 0 "
                     << "for short term storage " << name;
        return false;
    }

    if (efficiencyFactor < 0)
    {
        logs.warning() << "Property efficiency must be >= 0 "
                       << "for short term storage " << name;
        efficiencyFactor = 0;
    }

    if (efficiencyFactor > 1)
    {
        logs.warning() << "Property efficiency must be <= 1 "
                       << "for short term storage " << name;
        efficiencyFactor = 1;
    }

    if (initialLevel < 0)
    {
        initialLevel = initiallevelDefault;
        logs.warning() << "initiallevel for cluster: " << name << " should be positive, value has been set to " << initialLevel;

    }

    if (initialLevel > 1)
    {
        initialLevel = initiallevelDefault;
        logs.warning() << "initiallevel for cluster: " << name << " should be inferior to 1, value has been set to " << initialLevel;

    }

    return true;
}

} // namespace Antares::Data::ShortTermStorage
