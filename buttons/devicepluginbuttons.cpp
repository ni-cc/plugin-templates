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

#include "devicepluginbuttons.h"
#include "plugininfo.h"

// Note: You can find the tutorial for this code here -> http://dev.guh.guru/write-plugins.html

/* The constructor of this device plugin. */
DevicePluginButtons::DevicePluginButtons()
{
}

/* This method will be called from the devicemanager to get
 * information about this plugin which device resource will be needed.
 *
 * For multiple resources use the OR operator:
 * Example:
 *
 *  return DeviceManager::HardwareResourceTimer | DeviceManager::HardwareResourceNetworkManager;
 */
DeviceManager::HardwareResources DevicePluginButtons::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

/* This method will be called from the devicemanager while he
 * is setting up a new device. Here the developer has the chance to
 * perform the setup on the actual device and report the result.
 */
DeviceManager::DeviceSetupStatus DevicePluginButtons::setupDevice(Device *device)
{
    Q_UNUSED(device)
    qCDebug(dcButtons) << "Hello word! Setting up a new device:" << device->name();
    qCDebug(dcButtons) << "The new device has the DeviceId" << device->id().toString();
    qCDebug(dcButtons) << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

/* This method will be called whenever a client or the RuleEngine want's to execute
 * an action on the given device.
 */
DeviceManager::DeviceError DevicePluginButtons::executeAction(Device *device, const Action &action)
{
    // Tutorial 2
    // Check the DeviceClassId for "Simple Button"
    if (device->deviceClassId() == simpleButtonDeviceClassId ) {

        // check if this is the "press" action
        if (action.actionTypeId() == pressSimpleButtonActionTypeId) {

            qCDebug(dcButtons) << "Simple button" << device->paramValue("name").toString() << "was pressed";

            // Emit the "button pressed" event
            Event event(simpleButtonPressedEventTypeId, device->id());
            emit emitEvent(event);

            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }


    // Tutorial 3
    // Check the DeviceClassId for "Power Button"
    if (device->deviceClassId() == powerButtonDeviceClassId ) {

        // check if this is the "set power" action
        if (action.actionTypeId() == setPowerButtonActionTypeId) {

            // get the param value
            Param powerParam = action.param("power");
            bool power = powerParam.value().toBool();

            qCDebug(dcButtons) << "Power button" << device->paramValue("name").toString() << "set power to" << power;

            // Set the "power" state
            device->setStateValue(powerStateTypeId, power);

            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }


    // Tutorial 4
    // Check the DeviceClassId for "Alternative Power Button"
    if (device->deviceClassId() == alternativePowerButtonDeviceClassId) {

        // check if this is the "set power" action
        if (action.actionTypeId() == alternativePowerActionTypeId) {

            // get the param value
            Param powerParam = action.param("power");
            bool power = powerParam.value().toBool();

            qCDebug(dcButtons) << "Alternative power button" << device->paramValue("name").toString() << "set power to" << power;
            qCDebug(dcButtons) << "ActionTypeId :" << action.actionTypeId().toString();
            qCDebug(dcButtons) << "StateTypeId  :" << alternativePowerStateTypeId.toString();

            // Set the "power" state
            device->setStateValue(alternativePowerStateTypeId, power);

            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

