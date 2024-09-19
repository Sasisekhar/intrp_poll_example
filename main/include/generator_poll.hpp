#ifndef CADMIUM_EXAMPLE_GPT_GENERATOR_HPP_
#define CADMIUM_EXAMPLE_GPT_GENERATOR_HPP_

#include "cadmium/modeling/devs/atomic.hpp"
#include <iostream>
#include "mqtt/async_client.h"
#include "job.hpp"

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

	//! Class for representing the Generator DEVS model state.
	struct GeneratorState {
		double clock;  //!< Current simulation time.
		double sigma;  //!< Time to wait before triggering the next internal transition function.
		int jobCount;  //!< Number of jobs generated by the Generator model so far.

		//! Constructor function. It sets all the attributes to 0.
		GeneratorState(): clock(), sigma(), jobCount() {}
	};

	/**
	 * Insertion operator for GeneratorState objects. It only displays the value of jobCount.
	 * @param out output stream.
	 * @param s state to be represented in the output stream.
	 * @return output stream with jobCount already inserted.
	 */
	std::ostream& operator<<(std::ostream& out, const GeneratorState& s) {
		out << s.jobCount;
		return out;
	}

	//! Atomic DEVS model of a Job generator.
	class Generator : public Atomic<GeneratorState> {
	 private:
		double jobPeriod;                            //!< Time to wait between Job generations.
		mqtt::async_client cli;
		POLL_MessageCallback cb;
		POLL_MessageCallback* p_cb;
	 public:
		Port<bool> inStop;          //!< Input Port for receiving stop generating Job objects.
		BigPort<Job> outGenerated;  //!< Output Port for sending new Job objects to be processed.

		/**
		 * Constructor function for Generator DEVS model.
		 * @param id model ID.
		 * @param jobPeriod Job generation period.
		 */
		Generator(const std::string& id, double jobPeriod): Atomic<GeneratorState>(id, GeneratorState()), 
															jobPeriod(jobPeriod), 
															cli(POLL_SERVER_ADDRESS, POLL_CLIENT_ID), 
															cb(cli) {
			inStop = addInPort<bool>("inStop");
			outGenerated = addOutBigPort<Job>("outGenerated");

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
		 * Updates GeneratorState::clock and GeneratorState::sigma and increments GeneratorState::jobCount by one.
		 * @param s reference to the current generator model state.
		 */
		void internalTransition(GeneratorState& s) const override {
			if(this->p_cb->hasArrived()){
				s.jobCount += 1;
				this->p_cb->clearArrived();
			}

			s.clock += s.sigma;
			s.sigma = jobPeriod;
		}

		/**
		 * Updates GeneratorState::clock and GeneratorState::sigma.
		 * If it receives a true message via the Generator::inStop port, it passivates and stops generating Job objects.
		 * @param s reference to the current generator model state.
		 * @param e time elapsed since the last state transition function was triggered.
		 * @param x reference to the atomic model input port set.
		 */
		void externalTransition(GeneratorState& s, double e) const override {
			s.clock += e;
			s.sigma = std::max(s.sigma - e, 0.);
			if (!inStop->empty() && inStop->getBag().back()) {
				s.sigma = std::numeric_limits<double>::infinity();
			}
		}

		/**
		 * Sends a new Job that needs to be processed via the Generator::outGenerated port.
		 * @param s reference to the current generator model state.
		 * @param y reference to the atomic model output port set.
		 */
		void output(const GeneratorState& s) const override {
			if(this->p_cb->hasArrived()){
				outGenerated->addMessage(s.jobCount, s.clock + s.sigma);
			}
		}

		/**
		 * It returns the value of GeneratorState::sigma.
		 * @param s reference to the current generator model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_GENERATOR_HPP_