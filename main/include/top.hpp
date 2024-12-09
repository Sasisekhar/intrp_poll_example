#ifndef CADMIUM_EXAMPLE_EFP_GPT_HPP_
#define CADMIUM_EXAMPLE_EFP_GPT_HPP_

// #define INTERRUPT_ENABLED 1

#include "cadmium/modeling/devs/coupled.hpp"
#ifndef INTERRUPT_ENABLED
	#include "Poll_Subscriber.hpp"
#endif
#include "processor.hpp"
#include "MQTT_Publish.hpp"

namespace cadmium::example::gpt {
	//! Coupled DEVS model of the generator-processor-transducer.
	struct top : public Coupled {
		BigPort<int> in;

		top(const std::string& id): Coupled(id) {
			in = addInBigPort<int>("in");

			auto atomic1 = addComponent<Processor>("processor");
			auto atomic2 = addComponent<Publisher>("publisher");

			addCoupling(atomic1->out, atomic2->in);

			#ifdef INTERRUPT_ENABLED
				addCoupling(in, atomic1->in);
			#else
				auto atomic3 = addComponent<Poll_Subscriber>("poll_subscriber");
				addCoupling(atomic3->out, atomic1->in);
			#endif
		}
	};
}  //namespace cadmium::example::top

#endif //CADMIUM_EXAMPLE_EFP_GPT_HPP_
