#ifndef CADMIUM_EXAMPLE_GPT_Poll_Subscriber_HPP_
#define CADMIUM_EXAMPLE_GPT_Poll_Subscriber_HPP_

#include "cadmium/modeling/devs/atomic.hpp"
#include <iostream>
#include "mqtt/async_client.h"

#define POLL_RATE 1

const std::string POLL_SERVER_ADDRESS { "mqtt://broker.hivemq.com:1883" };
const std::string POLL_CLIENT_ID { "GENR_CONSUMER_ARSLAB_POLL" };
const std::string POLL_TOPIC { "ARSLAB/GPT" };
const int POLL_N_RETRY_ATTEMPTS = 5;
const int POLL_QOS = 1;

namespace cadmium::example::gpt {
	class POLL_MessageCallback : public virtual mqtt::callback, public virtual mqtt::iaction_listener {
		public:
			POLL_MessageCallback(mqtt::async_client& client) : cli(client), arrived(false) {}

			void connected(const std::string& cause) override {
				std::cout << "Connected (POLL)" << std::endl;
			}

			void connection_lost(const std::string& cause) override {
				std::cout << "Connection lost (POLL): " << cause << std::endl;
			}

			void message_arrived(mqtt::const_message_ptr msg) override {
				arrived = true;
				// std::cout << "Message arrived" << std::endl;
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
				std::cout << "\nConnection attempt failed (POLL)" << std::endl;
				if (++nretry > POLL_N_RETRY_ATTEMPTS)
					exit(1);
			}

			void on_success(const mqtt::token& tok) override {
				std::cout << "\nSuccessfully connected (POLL)" << std::endl;
			}

		private:
			mqtt::async_client& cli;
			int nretry {0};
			bool arrived;
	};

	//! Class for representing the Poll_Subscriber DEVS model state.
	struct Poll_SubscriberState {
		double sigma;

		//! Constructor function. It sets all the attributes to 0.
		Poll_SubscriberState(): sigma() {}
	};

	std::ostream& operator<<(std::ostream& out, const Poll_SubscriberState& s) {
		out << s.sigma;
		return out;
	}

	//! Atomic DEVS model of a Job Poll_Subscriber.
	class Poll_Subscriber : public Atomic<Poll_SubscriberState> {
	 private:
		mqtt::async_client cli;
		POLL_MessageCallback cb;
		POLL_MessageCallback* p_cb;
	 public:
		BigPort<int> out;

		Poll_Subscriber(const std::string& id): Atomic<Poll_SubscriberState>(id, Poll_SubscriberState()),
															cli(POLL_SERVER_ADDRESS, POLL_CLIENT_ID), 
															cb(cli) {
			out = addOutBigPort<int>("out");

			cli.set_callback(cb);
			
            auto connOpts = mqtt::connect_options_builder()
                .clean_session(true)
                .finalize();

            try {
                std::cout << "Connecting to the MQTT server (POLL)..." << std::endl;
                auto tok = cli.connect(connOpts);
                tok->wait();  // Wait for the connection to complete
                auto rsp = tok->get_connect_response();

                if (!rsp.is_session_present()) {
                    cli.subscribe(POLL_TOPIC, POLL_QOS)->wait();
                } else {
                    std::cout << "Session already present. Skipping subscription. (POLL)" << std::endl;
                }
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "\nERROR: Unable to connect to MQTT server (POLL): '"
                    << POLL_SERVER_ADDRESS << "' " << exc.what() << std::endl;
            }

			p_cb = &cb;
		}

		/**
		 * Updates Poll_SubscriberState::clock and Poll_SubscriberState::sigma and increments Poll_SubscriberState::jobCount by one.
		 * @param s reference to the current Poll_Subscriber model state.
		 */
		void internalTransition(Poll_SubscriberState& s) const override {
			if(this->p_cb->hasArrived()){
				this->p_cb->clearArrived();
			}
			s.sigma = POLL_RATE;
		}

		void externalTransition(Poll_SubscriberState& s, double e) const override {}

		void output(const Poll_SubscriberState& s) const override {
			if(this->p_cb->hasArrived()){
				out->addMessage(10);
			}
		}

		[[nodiscard]] double timeAdvance(const Poll_SubscriberState& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_Poll_Subscriber_HPP_
