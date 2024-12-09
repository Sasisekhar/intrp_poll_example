#ifndef MQTT_INTERRUPT_HANDLER_HPP
#define MQTT_INTERRUPT_HANDLER_HPP

#include "cadmium/simulation/rt_clock/interrupt_handler.hpp"
#include "mqtt/async_client.h"

const std::string SERVER_ADDRESS { "mqtt://broker.hivemq.com:1883" };
const std::string CLIENT_ID { "GENR_CONSUMER_ARSLAB" };
const std::string TOPIC { "ARSLAB/GPT" };
const int N_RETRY_ATTEMPTS = 5;
const int QOS = 1;

namespace cadmium::example::gpt {

class MessageCallback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {
    public:
        MessageCallback(mqtt::async_client& client) : cli(client), arrived(false) {}

        void connected(const std::string& cause) override {
            std::cout << "Connected" << std::endl;
        }

        void connection_lost(const std::string& cause) override {
            std::cout << "Connection lost: " << cause << std::endl;
        }

        void message_arrived(mqtt::const_message_ptr msg) override {
            arrived = true;
            std::cout << "Message Arrived" << std::endl;
        }

        bool hasArrived() {
            return arrived;
        }

        void clearArrived() {
            arrived = false;
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override {
            std::cout << "Delivery complete for token: " << (token ? token->get_message_id() : -1) << std::endl;
        }

        void on_failure(const mqtt::token& tok) override {
            std::cout << "\nConnection attempt failed" << std::endl;
            if (++nretry > N_RETRY_ATTEMPTS)
                exit(1);
        }

        void on_success(const mqtt::token& tok) override {
            std::cout << "\nSuccessfully connected" << std::endl;
        }

    private:
        mqtt::async_client& cli;
        int nretry {0};
        bool arrived;
};

class MQTTIntrHandler : public InterruptHandler<int> {
    private:
        mqtt::async_client cli;
        MessageCallback cb;
        int count;

    public:
        MQTTIntrHandler() : cli(SERVER_ADDRESS, CLIENT_ID), cb(cli) {
            count = 0;
            cli.set_callback(cb);

            auto connOpts = mqtt::connect_options_builder()
                .clean_session(true)
                .finalize();

            try {
                std::cout << "Connecting to the MQTT server..." << std::endl;
                auto tok = cli.connect(connOpts);
                tok->wait();  // Wait for the connection to complete
                auto rsp = tok->get_connect_response();

                if (!rsp.is_session_present()) {
                    cli.subscribe(TOPIC, QOS)->wait();
                } else {
                    std::cout << "Session already present. Skipping subscription." << std::endl;
                }
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "\nERROR: Unable to connect to MQTT server: '"
                    << SERVER_ADDRESS << "' " << exc.what() << std::endl;
            }
        }

        bool ISRcb() {
            if(cb.hasArrived()) {
                cb.clearArrived();
                return true;
            }
            return false;
        }

        int decodeISR() {
            return 10;
        }
    };
}

#endif
