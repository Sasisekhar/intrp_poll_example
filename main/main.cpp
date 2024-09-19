#include "include/cadmium/simulation/rt_root_coordinator.hpp"
#include "include/gpt.hpp"
#include "include/genr_interrupt_handler.hpp"
#include "include/job.hpp"
#include <include/cadmium/simulation/rt_clock/chrono.hpp>

#ifndef NO_LOGGING
	#include "include/cadmium/simulation/logger/stdout.hpp"
	#include "include/cadmium/simulation/logger/csv.hpp"
#endif

using namespace cadmium::example::gpt;

int main() {

	double observation_time = 20.0;

	std::shared_ptr<GPT> model = std::make_shared<GPT> ("gpt", 3, 0.5, observation_time);

	cadmium::ChronoClock<std::chrono::steady_clock, Job, cadmium::example::gpt::GenrIntrHandler> clock(model);
	auto rootCoordinator = cadmium::RealTimeRootCoordinator<
															cadmium::ChronoClock<
																std::chrono::steady_clock, 
																Job, 
																cadmium::example::gpt::GenrIntrHandler
																>
															> (model, clock);

	#ifndef NO_LOGGING
	rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
	#endif

	rootCoordinator.start();

	rootCoordinator.simulate(observation_time + 1.0);

	rootCoordinator.stop();	

	return 0;
}

