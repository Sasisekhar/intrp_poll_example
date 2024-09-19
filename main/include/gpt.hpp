#ifndef CADMIUM_EXAMPLE_EFP_GPT_HPP_
#define CADMIUM_EXAMPLE_EFP_GPT_HPP_

// #define INTERRUPT_ENABLED 1

#include "cadmium/modeling/devs/coupled.hpp"
#ifndef INTERRUPT_ENABLED
	#include "generator_poll.hpp"
#endif
#include "processor.hpp"
#include "transducer.hpp"

namespace cadmium::example::gpt {
	//! Coupled DEVS model of the generator-processor-transducer.
	struct GPT : public Coupled {
		BigPort<Job> in;
		/**
		 * Constructor function for the GPT model.
		 * @param id ID of the gpt model.
		 * @param jobPeriod Job generation period for the Generator model.
		 * @param processingTime Job processing time for the Processor model.
		 * @param obsTime time to wait by the Transducer before asking the Generator to stop creating Job objects.
		 */
		GPT(const std::string& id, double jobPeriod, double processingTime, double obsTime): Coupled(id) {
			in = addInBigPort<Job>("in");

			auto processor = addComponent<Processor>("processor", processingTime);
			auto transducer = addComponent<Transducer>("transducer", obsTime);

			addCoupling(processor->outProcessed, transducer->inProcessed);

			#ifdef INTERRUPT_ENABLED
				addCoupling(in, processor->inGenerated);
				addCoupling(in, transducer->inGenerated);
			#else
				auto generator = addComponent<Generator>("generator", jobPeriod);
				addCoupling(generator->outGenerated, processor->inGenerated);
				addCoupling(generator->outGenerated, transducer->inGenerated);
			#endif
		}
	};
}  //namespace cadmium::example::gpt

#endif //CADMIUM_EXAMPLE_EFP_GPT_HPP_
