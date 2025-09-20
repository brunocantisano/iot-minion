#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <HTTPClient.h>
#include <AdafruitIO_WiFi.h>
#include "HttpStatusCodes.h"

class MqttHandler {

private:
    AdafruitIO_WiFi * io;
    AdafruitIO_Feed * feed;
    bool              conectado = false;
    String            aio_username;
    String            aio_key;

public:
    MqttHandler(const String& aio_username,
                const String& aio_key,
                const String& ssid,
                const String& passwd);
    ~MqttHandler();

    // deve ser chamado periodicamente no loop
    void update();
    bool isConnected() const;

    // publica no feed “cavaloX” e “status”
    void setFeed(const String& name, const String& value);

    // checa existência de feed
    bool adafruitFeedExists(const String& feedName,
                            const String& username,
                            const String& aioKey) const;

    bool createAdafruitFeed(const String& feedName,
                            const String& aioKey) const;
};

#endif