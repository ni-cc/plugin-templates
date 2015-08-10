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

#include "devicepluginnetworkinfo.h"
#include "plugininfo.h"

#include <QJsonDocument>

// Note: You can find the documentation for this code here -> http://dev.guh.guru/write-plugins.html

// The constructor of this device plugin.
DevicePluginNetworkInfo::DevicePluginNetworkInfo()
{
}

DeviceManager::HardwareResources DevicePluginNetworkInfo::requiredHardware() const
{
    return DeviceManager::HardwareResourceNetworkManager;
}

DeviceManager::DeviceSetupStatus DevicePluginNetworkInfo::setupDevice(Device *device)
{
    Q_UNUSED(device)
    qCDebug(dcNetworkInfo) << "Setting up a new device:" << device->name() << device->id();
    qCDebug(dcNetworkInfo) << device->params();

    return DeviceManager::DeviceSetupStatusSuccess;
}

// This method will be called whenever the reply from a NetworkManager call is ready.
void DevicePluginNetworkInfo::networkManagerReplyReady(QNetworkReply *reply)
{
    // Make shore this is our reply
    if (!m_asyncActionReplies.keys().contains(reply))
        return;

    // This is one of our action replies!!

    // Take the corresponding action from our hash
    ActionId actionId = m_asyncActionReplies.take(reply);

    // Check the status code of the reply
    if (reply->error()) {

        // Print the warning message
        qCWarning(dcNetworkInfo) << "Reply error" << reply->errorString();

        // The action execution is finished, and was not successfully
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareNotAvailable);

        // Important -> delete the reply to prevent a memory leak!
        reply->deleteLater();
        return;
    }

    // The request was successfull, lets read the payload
    QByteArray data = reply->readAll();

    // Important -> delete the reply to prevent a memory leak!
    reply->deleteLater();

    // Process the data from the reply
    actionDataReady(actionId, data);
}

// This method will be called whenever a client or the rule engine wants to execute an action for the given device.
DeviceManager::DeviceError DevicePluginNetworkInfo::executeAction(Device *device, const Action &action)
{
    // check if this device is a Network info device using the DeviceClassId
    if (device->deviceClassId() != infoDeviceClassId) {
        return DeviceManager::DeviceErrorDeviceClassNotFound;
    }

    // check if the requested action is our "update" action ...
    if (action.actionTypeId() == updateActionTypeId) {

        // Print information that we are executing now the update action
        qCDebug(dcNetworkInfo) << "Execute update action" << action.id();

        // Create a network request
        QNetworkRequest locationRequest(QUrl("http://ip-api.com/json"));

        // Call the GET method from the NetworkManager
        QNetworkReply *reply = networkManagerGet(locationRequest);

        // Hash the reply, because we dont get the result immediately
        m_asyncActionReplies.insert(reply, action.id());

        // Hash the device for this action
        m_asyncActions.insert(action.id(), device);

        // Tell the DeviceManager that this is an async action and the result of the execution will
        // be emitted later.
        return DeviceManager::DeviceErrorAsync;
    }

    // ...otherwise the ActionType does not exist
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

void DevicePluginNetworkInfo::actionDataReady(const ActionId &actionId, const QByteArray &data)
{
    // Convert the rawdata to a json document
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);

    // Check if we got a valid JSON document
    if(error.error != QJsonParseError::NoError) {
        qCWarning(dcNetworkInfo) << "Failed to parse JSON data" << data << ":" << error.errorString();

        // the action execution is finished, and was not successfully
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
        return;
    }

    // print the fetched data in json format to stdout
    qCDebug(dcNetworkInfo) << jsonDoc.toJson();

    // Get the device for this action
    Device *device = m_asyncActions.take(actionId);

    // Parse the data and update the states of our device
    QVariantMap dataMap = jsonDoc.toVariant().toMap();

    // Set the city state
    if (dataMap.contains("city")) {
        device->setStateValue(cityStateTypeId, dataMap.value("city").toString());
    }

    // Set the country state
    if (dataMap.contains("countryCode")) {
        device->setStateValue(countryStateTypeId, dataMap.value("countryCode").toString());
    }

    // Set the wan ip
    if (dataMap.contains("query")) {
        device->setStateValue(addressStateTypeId, dataMap.value("query").toString());
    }

    // Set the time zone state
    if (dataMap.contains("timezone")) {
        device->setStateValue(timeZoneStateTypeId, dataMap.value("timezone").toString());
    }

    // Set the longitude state
    if (dataMap.contains("lon")) {
        device->setStateValue(lonStateTypeId, dataMap.value("lon").toDouble());
    }

    // Set the latitude state
    if (dataMap.contains("lat")) {
        device->setStateValue(latStateTypeId, dataMap.value("lat").toDouble());
    }

    qCDebug(dcNetworkInfo) << "Action" << actionId << "execution finished successfully.";

    // Emit the successfull action execution result to the device manager
    emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);
}

