#pragma once
#include "3rdparty/QJsonStruct/QJsonStruct.hpp"
#include "QvCoreSettings.hpp"

#include <QHash>
#include <QHashFunctions>
#include <QString>
#include <QtCore>

namespace Qv2ray::base
{
    template<typename T>
    class IDType
    {
      public:
        explicit IDType() : m_id("null"){};
        explicit IDType(const QString &id) : m_id(id){};
        friend bool operator==(const IDType<T> &lhs, const IDType<T> &rhs)
        {
            return lhs.m_id == rhs.m_id;
        }
        friend bool operator!=(const IDType<T> &lhs, const IDType<T> &rhs)
        {
            return lhs.m_id != rhs.m_id;
        }
        const QString toString() const
        {
            return m_id;
        }
        void loadJson(const QJsonValue &d)
        {
            m_id = d.toString("null");
        }
        QJsonValue toJson() const
        {
            return m_id;
        }

      private:
        QString m_id;
    };

    // Define several safetypes to prevent misuse of QString.
#define DECL_IDTYPE(type)                                                                                                                            \
    class __##type;                                                                                                                                  \
    typedef IDType<__##type> type

    DECL_IDTYPE(GroupId);
    DECL_IDTYPE(ConnectionId);
    DECL_IDTYPE(GroupRoutingId);
    //
    inline const static ConnectionId NullConnectionId;
    inline const static GroupId NullGroupId;
    inline const static GroupRoutingId NullRoutingId;
    //
    class ConnectionGroupPair
    {
      public:
        ConnectionGroupPair() : connectionId(NullConnectionId), groupId(NullGroupId){};
        ConnectionGroupPair(const ConnectionId &conn, const GroupId &group) : connectionId(conn), groupId(group){};
        Q_PROPERTY(ConnectionId connectionId MEMBER)
        Q_PROPERTY(GroupId groupId MEMBER)
        //
        ConnectionId connectionId = NullConnectionId;
        GroupId groupId = NullGroupId;

      public slots:
        void clear()
        {
            connectionId = NullConnectionId;
            groupId = NullGroupId;
        }

      public:
        bool isEmpty() const
        {
            return groupId == NullGroupId || connectionId == NullConnectionId;
        }
        friend bool operator==(const ConnectionGroupPair &lhs, const ConnectionGroupPair &rhs)
        {
            return lhs.groupId == rhs.groupId && lhs.connectionId == rhs.connectionId;
        }
        friend bool operator!=(const ConnectionGroupPair &lhs, const ConnectionGroupPair &rhs)
        {
            return !(lhs == rhs);
        }
        JSONSTRUCT_REGISTER(ConnectionGroupPair, F(connectionId, groupId))
    };
    //
    constexpr unsigned int LATENCY_TEST_VALUE_ERROR = 99999;
    constexpr unsigned int LATENCY_TEST_VALUE_NODATA = LATENCY_TEST_VALUE_ERROR - 1;
    using namespace std::chrono;

    struct __Qv2rayConfigObjectBase
    {
        QString displayName;
        qint64 creationDate = system_clock::to_time_t(system_clock::now());
        qint64 lastUpdatedDate = system_clock::to_time_t(system_clock::now());
        JSONSTRUCT_REGISTER(__Qv2rayConfigObjectBase, F(displayName, creationDate, lastUpdatedDate))
    };

    struct GroupRoutingConfig : __Qv2rayConfigObjectBase
    {
        bool overrideDNS = false;
        config::QvConfig_DNS dnsConfig;
        //
        bool overrideRoute = false;
        config::QvConfig_Route routeConfig;
        //
        bool overrideConnectionConfig = false;
        config::QvConfig_Connection connectionConfig;
        //
        bool overrideForwardProxyConfig = false;
        config::QvConfig_ForwardProxy forwardProxyConfig;
        //
        JSONSTRUCT_REGISTER(GroupRoutingConfig,                            //
                            F(overrideRoute, routeConfig),                 //
                            F(overrideDNS, dnsConfig),                     //
                            F(overrideConnectionConfig, connectionConfig), //
                            F(overrideForwardProxyConfig, forwardProxyConfig))
    };

    enum SubscriptionFilterRelation
    {
        RELATION_AND = 0,
        RELATION_OR = 1
    };

    struct SubscriptionConfigObject
    {
        QString address;
        QString type = "simple_base64";
        float updateInterval = 10;
        QList<QString> IncludeKeywords;
        QList<QString> ExcludeKeywords;
        SubscriptionFilterRelation IncludeRelation = RELATION_OR;
        SubscriptionFilterRelation ExcludeRelation = RELATION_AND;
        JSONSTRUCT_REGISTER(SubscriptionConfigObject,
                            F(updateInterval, address, type, IncludeRelation, ExcludeRelation, IncludeKeywords, ExcludeKeywords))
    };

    struct GroupObject : __Qv2rayConfigObjectBase
    {
        bool isSubscription = false;
        QList<ConnectionId> connections;
        GroupRoutingId routeConfigId;
        SubscriptionConfigObject subscriptionOption;
        GroupObject() : __Qv2rayConfigObjectBase(){};
        JSONSTRUCT_REGISTER(GroupObject, F(connections, isSubscription, routeConfigId, subscriptionOption), B(__Qv2rayConfigObjectBase))
    };

    enum StatisticsType
    {
        API_INBOUND = 0,
        API_OUTBOUND_PROXY = 1,
        API_OUTBOUND_DIRECT = 2,
        API_OUTBOUND_BLACKHOLE = 3,
    };

#define CurrentStatAPIType (GlobalConfig.uiConfig.graphConfig.useOutboundStats ? API_OUTBOUND_PROXY : API_INBOUND)

    typedef long qvspeed;
    typedef quint64 qvdata;
    typedef QPair<qvspeed, qvspeed> QvStatsSpeed;
    typedef QPair<qvdata, qvdata> QvStatsData;
    typedef QPair<QvStatsSpeed, QvStatsData> QvStatsSpeedData;

    struct ConnectionStatsEntryObject
    {
        qvdata upLinkData;
        qvdata downLinkData;
        QvStatsData toData()
        {
            return { upLinkData, downLinkData };
        }
        void fromData(const QvStatsData &d)
        {
            upLinkData = d.first;
            downLinkData = d.second;
        }
        JSONSTRUCT_REGISTER(ConnectionStatsEntryObject, F(upLinkData, downLinkData))
    };

    enum ConnectionImportSource
    {
        IMPORT_SOURCE_SUBSCRIPTION,
        IMPORT_SOURCE_MANUAL
    };

    struct ConnectionStatsObject
    {
        ConnectionStatsEntryObject &operator[](StatisticsType i)
        {
            while (entries.count() <= i)
            {
                entries.append(ConnectionStatsEntryObject{});
            }
            return entries[i];
        }
        QJsonValue toJson() const
        {
            return JsonStructHelper::Serialize(entries);
        }
        void loadJson(const QJsonValue &d)
        {
            entries.clear();
            JsonStructHelper::Deserialize(entries, d);
        }
        void Clear()
        {
            entries.clear();
        }

      private:
        QList<ConnectionStatsEntryObject> entries;
    };

    struct ConnectionObject : __Qv2rayConfigObjectBase
    {
        qint64 lastConnected;
        qint64 latency = LATENCY_TEST_VALUE_NODATA;
        ConnectionImportSource importSource = IMPORT_SOURCE_MANUAL;
        ConnectionStatsObject stats;
        //
        int __qvConnectionRefCount = 0;
        JSONSTRUCT_REGISTER(ConnectionObject, F(lastConnected, latency, importSource, stats), B(__Qv2rayConfigObjectBase))
    };

    struct ProtocolSettingsInfoObject
    {
        QString protocol;
        QString address;
        int port;
        ProtocolSettingsInfoObject(){};
        ProtocolSettingsInfoObject(const QString &_protocol, const QString _address, int _port)
            : protocol(_protocol), //
              address(_address),   //
              port(_port)          //
              {};
        JSONSTRUCT_REGISTER(ProtocolSettingsInfoObject, F(protocol, address, port))
    };
    //

    template<typename T>
    inline uint qHash(IDType<T> key)
    {
        return ::qHash(key.toString());
    }
    inline uint qHash(const Qv2ray::base::ConnectionGroupPair &pair)
    {
        return ::qHash(pair.connectionId.toString() + pair.groupId.toString());
    }
} // namespace Qv2ray::base

using namespace Qv2ray::base;
Q_DECLARE_METATYPE(ConnectionGroupPair)
Q_DECLARE_METATYPE(ConnectionId)
Q_DECLARE_METATYPE(GroupId)
Q_DECLARE_METATYPE(GroupRoutingId)

Q_DECLARE_METATYPE(QvStatsSpeed)
Q_DECLARE_METATYPE(QvStatsData)
Q_DECLARE_METATYPE(QvStatsSpeedData)
Q_DECLARE_METATYPE(StatisticsType)
