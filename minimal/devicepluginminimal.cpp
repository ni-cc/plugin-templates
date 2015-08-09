/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "devicepluginminimal.h"
#include "plugininfo.h"

/* The constructor of this device plugin. */
DevicePluginMinimal::DevicePluginMinimal()
{
}

/* This method will be called from the devicemanager to get
 * information about this plugin which device resource will be needed.
 *
 * For multiple resources use the OR operator:
 * Example:
 *
 *  return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceNetworkManager
 *
 */
DeviceManager::HardwareResources DevicePluginMinimal::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

/* This method will be called from the devicemanager while he
 * is setting up a new device. Here the developer has the chance to
 * perform the setup on the actual device and report the result.
 */
DeviceManager::DeviceSetupStatus DevicePluginMinimal::setupDevice(Device *device)
{
    Q_UNUSED(device)
    qCDebug(dcMinimal) << "Hello word! Setting up a new device:" << device->name();
    qCDebug(dcMinimal) << "The new device has the DeviceId" << device->id().toString();
    qCDebug(dcMinimal) << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

