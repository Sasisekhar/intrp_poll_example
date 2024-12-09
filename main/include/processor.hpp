#ifndef CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_
#define CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_

#include "cadmium/modeling/devs/atomic.hpp"
#include <iostream>

namespace cadmium::example::gpt {
	//! Class for representing the Processor DEVS model state.
	struct ProcessorState {
		double sigma;                   //!< Time to wait before triggering the next internal transition function.

		//! Processor state constructor. By default, the processor is idling.
		ProcessorState(): sigma(std::numeric_limits<double>::infinity()) {}
	};

	std::ostream& operator<<(std::ostream &out, const ProcessorState& s) {
		out << s.sigma;
		return out;
	}

	//! Atomic DEVS model of a int processor.
	class Processor : public Atomic<ProcessorState> {
	 public:
		BigPort<int> in;  //!< Input Port for receiving new int objects.
		BigPort<int> out;  //!< Output Port for sending processed int objects.

		Processor(const std::string& id): Atomic<ProcessorState>(id, ProcessorState()) {
			in = addInBigPort<int>("in");
			out = addOutBigPort<int>("out");
		}

		void internalTransition(ProcessorState& s) const override {
			s.sigma = std::numeric_limits<double>::infinity();
		}

		void externalTransition(ProcessorState& s, double e) const override {
			if(!in->empty()) {
				for(const auto &x : in->getBag()) {
					s.sigma = 0.5;
				}
			}
		}

		void output(const ProcessorState& s) const override {
			out->addMessage(10);
		}

		[[nodiscard]] double timeAdvance(const ProcessorState& s) const override {
			return s.sigma;
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_GPT_PROCESSOR_HPP_
