#pragma once

#include "base/Qv2rayBase.hpp"

#include <QObject>
namespace Qv2ray::core::handler
{
    class RouteHandler : public QObject
    {
        Q_OBJECT
      public:
        explicit RouteHandler(QObject *parent = nullptr);
        ~RouteHandler();
        void SaveRoutes() const;
        //
        QPair<bool, QvConfig_DNS> GetDNSSettings(const GroupRoutingId &id) const
        {
            return { configs[id].overrideDNS, configs[id].dnsConfig };
        }
        QPair<bool, QvConfig_Route> GetAdvancedRoutingSettings(const GroupRoutingId &id) const
        {
            return { configs[id].overrideRoute, configs[id].routeConfig };
        }
        //
        bool SetDNSSettings(const GroupRoutingId &id, bool overrideGlobal, const QvConfig_DNS &dns);
        bool SetAdvancedRouteSettings(const GroupRoutingId &id, bool overrideGlobal, const QvConfig_Route &dns);
        //
        OUTBOUNDS ExpandExternalConnection(const OUTBOUNDS &outbounds) const;
        //
        // Final Config Generation
        CONFIGROOT GenerateFinalConfig(const ConnectionGroupPair &pair, bool hasAPI = true) const;
        CONFIGROOT GenerateFinalConfig(CONFIGROOT root, const GroupRoutingId &routingId, bool hasAPI = true) const;
        //
        OUTBOUNDS ExpandChainedOutbounds() const;

      private:
        QHash<GroupRoutingId, GroupRoutingConfig> configs;
    };
    inline ::Qv2ray::core::handler::RouteHandler *RouteManager = nullptr;
} // namespace Qv2ray::core::handler
