/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#include "deviceplugincoapclient.h"
#include "plugininfo.h"

#include <QJsonDocument>

#include "coap/corelinkparser.h"

// Note: You can find the documentation for this code here -> http://dev.guh.guru/write-plugins.html

// The constructor of this device plugin.
DevicePluginCoapClient::DevicePluginCoapClient()
{
}

DeviceManager::HardwareResources DevicePluginCoapClient::requiredHardware() const
{
    return DeviceManager::HardwareResourceNone;
}

DeviceManager::DeviceSetupStatus DevicePluginCoapClient::setupDevice(Device *device)
{
    // Check if we already have a coap client device
    if (!myDevices().isEmpty()) {
        qCWarning(dcCoapClient) << "There is already a configured coap client device";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    qCDebug(dcCoapClient) << "Setting up a new device:" << device->name() << device->params();

    // Verify the given URL
    QUrl url(device->paramValue("url").toString());
    if (url.scheme() != "coap") {
        qCWarning(dcCoapClient) << "Invalid URL scheme" << url.scheme() << " != " << "coap";
        return DeviceManager::DeviceSetupStatusFailure;
    }

    m_device = device;

    // Create new CoAP client if there isn't one yet
    if (m_coap.isNull()) {
        m_coap = new Coap(this);
        connect(m_coap, &Coap::replyFinished, this, &DevicePluginCoapClient::onReplyFinished);
        connect(m_coap, &Coap::notificationReceived, this, &DevicePluginCoapClient::onNotificationReceived);
    }

    // Discover the CoAP server
    url.setPath("/.well-known/core");
    CoapReply *reply = m_coap->get(CoapRequest(url));

    // Check imediatly if the there occured any error
    if (reply->error() != CoapReply::NoError) {
        qCWarning(dcCoapClient) << "Could not discover CoAP server:" << reply->errorString();
        reply->deleteLater();
        m_coap->deleteLater();
        return DeviceManager::DeviceSetupStatusFailure;
    }

    // Store the reply and device until we get our asynchronous response
    m_discoverReplies.insert(reply, device);

    // Tell the DeviceManager that the setup result will be communicated later
    return DeviceManager::DeviceSetupStatusAsync;
}

void DevicePluginCoapClient::deviceRemoved(Device *device)
{
    // Prevent the unused variable warning
    Q_UNUSED(device)

    // Delete the CoAP socket if not longer needed
    m_coap->deleteLater();
}

// This method will be called whenever a client or the rule engine wants to execute an action for the given device.
DeviceManager::DeviceError DevicePluginCoapClient::executeAction(Device *device, const Action &action)
{
    qCDebug(dcCoapClient) << "Execute action" << action.id() << action.params();

    // check if the requested action is our "upload" action ...
    if (action.actionTypeId() == notificationsActionTypeId) {

        // observe resource (enable notifications)
        QUrl url(device->paramValue("url").toString());
        url.setPath(url.path().append("/obs"));

        if (action.param("notification").value().toBool()) {
            qCDebug(dcCoapClient) << "Enable notification on resource" << url.toString();
            CoapReply *reply = m_coap->enableResourceNotifications(CoapRequest(url));
            m_asyncActions.insert(reply, action.id());
            m_notificationEnableReplies.insert(reply, device);
        } else {
            qCDebug(dcCoapClient) << "Disable notification on resource" << url.toString();
            CoapReply *reply = m_coap->disableNotifications(CoapRequest(url));
            m_asyncActions.insert(reply, action.id());
            m_notificationDisableReplies.insert(reply, device);
        }

        // Tell the DeviceManager that this is an async action and the
        // result of the execution will be emitted later.
        return DeviceManager::DeviceErrorAsync;

    } else if (action.actionTypeId() == uploadActionTypeId) {

        // Define the URL for uploading the message (POST)
        QUrl url(device->paramValue("url").toString());
        url.setPath(url.path().append("/test"));

        // Upload the message (POST)
        CoapReply *reply = m_coap->post(CoapRequest(url), action.param("message").value().toString().toUtf8());
        m_uploadReplies.append(reply);
        m_asyncActions.insert(reply, action.id());

        // Tell the DeviceManager that this is an async action and the
        // result of the execution will be emitted later.
        return DeviceManager::DeviceErrorAsync;
    }

    // ...otherwise the ActionType does not exist
    return DeviceManager::DeviceErrorActionTypeNotFound;
}

// This slot will be called whenever a reply from the CoAP socket has finished
void DevicePluginCoapClient::onReplyFinished(CoapReply *reply)
{
    // Now check which reply this was by checking in which Hash it can be found
    if (m_discoverReplies.keys().contains(reply)) {
        Device *device = m_discoverReplies.take(reply);

        // Verify there where no reply errors (transport layer)
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcCoapClient) << "CoAP resource discovery reply error" << reply->errorString();
            reply->deleteLater();
            // Something went wrong during the discovery. Finish the setup with error.
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            return;
        }

        // Verify we have the right status code (server response)
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcCoapClient) << "CoAP discovery status code:" << reply;
            reply->deleteLater();
            // Something went wrong during the discovery. Finish the setup with error.
            emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusFailure);
            return;
        }

        qCDebug(dcCoapClient) << "Discovered successfully the resources";

        // Print the CoRE links we got from the server resource discovery
        CoreLinkParser parser(reply->payload());
        foreach (const CoreLink &link, parser.links()) {
            qCDebug(dcCoapClient) << link << endl;
        }

        // Tell the device manager that the device setup finished successfully
        emit deviceSetupFinished(device, DeviceManager::DeviceSetupStatusSuccess);

    } else if (m_notificationEnableReplies.keys().contains(reply)) {
        Device *device = m_notificationEnableReplies.take(reply);
        ActionId actionId = m_asyncActions.take(reply);

        // Verify there where no reply errors (transport layer)
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcCoapClient) << "CoAP enable observe resource reply error" << reply->errorString();
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        // Verify we have the right status code (server response)
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcCoapClient) << "CoAP enable observe status code:" << reply;
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        qCDebug(dcCoapClient) << "Enabled successfully notifications" << reply;

        // Set the corresping state
        device->setStateValue(notificationsStateTypeId, true);

        // Tell the device manager that the action execution finished successfully
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);

    } else if (m_notificationDisableReplies.keys().contains(reply)) {
        Device *device = m_notificationDisableReplies.take(reply);
        ActionId actionId = m_asyncActions.take(reply);

        // Verify there where no reply errors (transport layer)
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcCoapClient) << "CoAP disable observe resource reply error" << reply->errorString();
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        // Verify we have the right status code (server response)
        if (reply->statusCode() != CoapPdu::Content) {
            qCWarning(dcCoapClient) << "CoAP disable observe status code:" << reply;
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        qCDebug(dcCoapClient) << "Disabled successfully notifications" << reply;

        // Set the corresping state
        device->setStateValue(notificationsStateTypeId, false);

        // Tell the device manager that the action execution finished successfully
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);

    } else if (m_uploadReplies.contains(reply)) {
        ActionId actionId = m_asyncActions.take(reply);

        // Verify there where no reply errors (transport layer)
        if (reply->error() != CoapReply::NoError) {
            qCWarning(dcCoapClient) << "CoAP upload reply error" << reply->errorString();
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        // Verify we have the right status code (server response)
        if (reply->statusCode() != CoapPdu::Created) {
            qCWarning(dcCoapClient) << "CoAP upload status code:" << reply;
            // Something went wrong. Tell the devicemanager that the action finished with error.
            emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorHardwareFailure);
            reply->deleteLater();
            return;
        }

        qCDebug(dcCoapClient) << "Uploaded message successfully" << reply;

        // Tell the device manager that the action execution finished successfully
        emit actionExecutionFinished(actionId, DeviceManager::DeviceErrorNoError);

    }

    // Always make sure the reply will be deleted
    reply->deleteLater();
}

// This method will be called if the CoAP socket received a notification from an observed resource
void DevicePluginCoapClient::onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload)
{
    qCDebug(dcCoapClient) << "Got notification from observed resource" << notificationNumber << resource.url().path() << endl << payload;

    // Create the params for the event
    ParamList paramList;
    paramList.append(Param("time", payload));

    // Tell the device manager we got an event
    emitEvent(Event(timeEventTypeId, m_device->id(), paramList));
}


