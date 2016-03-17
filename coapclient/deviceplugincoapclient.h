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

#ifndef DEVICEPLUGINCOAPCLIENT_H
#define DEVICEPLUGINCOAPCLIENT_H

#include "devicemanager.h"
#include "plugin/deviceplugin.h"
#include "coap/coap.h"

#include <QHash>
#include <QNetworkReply>

class DevicePluginCoapClient : public DevicePlugin
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugincoapclient.json")
    Q_INTERFACES(DevicePlugin)

public:
    explicit DevicePluginCoapClient();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;

    // Will be called from the device manager once the user removes a configured device
    void deviceRemoved(Device *device) override;

    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

private:
    QPointer<Device> m_device;
    QPointer<Coap> m_coap;

    // Replies from coap
    QHash<CoapReply *, Device *> m_discoverReplies;
    QHash<CoapReply *, Device *> m_notificationEnableReplies;
    QHash<CoapReply *, Device *> m_notificationDisableReplies;
    QList<CoapReply *> m_uploadReplies;

    QHash< CoapReply *, ActionId> m_asyncActions;

private slots:
    void onReplyFinished(CoapReply *reply);
    void onNotificationReceived(const CoapObserveResource &resource, const int &notificationNumber, const QByteArray &payload);

};

#endif // DEVICEPLUGINNETWORKINFO_H
