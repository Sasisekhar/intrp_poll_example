#ifndef CADMIUM_EXAMPLE_GPT_Publisher_HPP_
#define CADMIUM_EXAMPLE_GPT_Publisher_HPP_

#include "cadmium/modeling/devs/atomic.hpp"
#include <iostream>
#include <optional>

namespace cadmium::example::gpt {
	//! Class for representing the Publisher DEVS model state.
	struct PublisherState {
		double sigma;
		//! Publisher state constructor. By default, the Publisher is idling.
		PublisherState(): sigma(std::numeric_limits<double>::infinity()) {}
	};

	std::ostream& operator<<(std::ostream &out, const PublisherState& s) {
		out << s.sigma;
		return out;
	}

	//! Atomic DEVS model of a int Publisher.
	class Publisher : public Atomic<PublisherState> {
	 public:
		BigPort<int> in;

		Publisher(const std::string& id): Atomic<PublisherState>(id, PublisherState()){
			in = addInBigPort<int>("in");
		}

		void internalTransition(PublisherState& s) const override {
			s.sigma = std::numeric_limits<double>::infinity();
		}

		void externalTransition(PublisherState& s, double e) const override {
			s.sigma = 0;
		}

		void output(const PublisherState& s) const override {}

		/**
		 * It returns the value of PublisherState::sigma.
		 * @param s reference to the current model state.
		 * @return the sigma value.
		 */
		[[nodiscard]] double timeAdvance(const PublisherState& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_Publisher_HPP_
