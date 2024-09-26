#include <string>
#include <iostream>
#include <mqtt/async_client.h>

#include "mqtt_client.h"

namespace digital_twin {
    class Mqtt_client_callback : public virtual mqtt::callback {
        public:
            void connected(const std::string &cause) override {
                std::cout << "Connected: " << cause << std::endl;
            }

            void connection_lost(const std::string &cause) override {
                std::cout << "Connection lost: " << cause << std::endl;
            }
    };

    void Mqtt_client::connect() {
        std::string server_address = "b09b50fd24a44677bcd84f0669c31841.s1.eu.hivemq.cloud";
        std::string client_id = "test_id";

        mqtt::async_client client(server_address, client_id);

        mqtt::connect_options connOpts;
        connOpts.set_keep_alive_interval(20);
        connOpts.set_clean_session(true);

        Mqtt_client_callback callback;
        client.set_callback(callback);

        mqtt::token_ptr connectionToken = client.connect(connOpts);
        connectionToken->wait();
    }
}


