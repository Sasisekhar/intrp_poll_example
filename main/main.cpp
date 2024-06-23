#include "include/cadmium/simulation/rt_root_coordinator.hpp"
#include "include/cadmium/simulation/root_coordinator.hpp"
#include <limits>
#include "include/gpt.hpp"
#include "include/genr_interrupt.hpp"
#include "include/job.hpp"
#ifdef RT_ESP32
	#include <include/cadmium/simulation/rt_clock/ESPclock.hpp>
#else
	#include <include/cadmium/simulation/rt_clock/chrono.hpp>
#endif
#ifndef NO_LOGGING
	#include "include/cadmium/simulation/logger/stdout.hpp"
	#include "include/cadmium/simulation/logger/csv.hpp"
#endif

using namespace cadmium::example::gpt;

extern "C" {
	#ifdef RT_ESP32
		void app_main() //starting point for ESP32 code
	#else
		int main()		//starting point for simulation code
	#endif
	{

		std::shared_ptr<GPT> model = std::make_shared<GPT> ("gpt", 10, 0.5, 10);

		#ifdef RT_ESP32
			cadmium::ESPclock clock;
			auto rootCoordinator = cadmium::RealTimeRootCoordinator<cadmium::ESPclock<double>>(model, clock);
		#else
			cadmium::ChronoClock<std::chrono::steady_clock, Job, cadmium::example::gpt::GenrIntrHandler> clock(model);
			auto rootCoordinator = cadmium::RealTimeRootCoordinator<
															cadmium::ChronoClock<
																std::chrono::steady_clock, 
																Job, 
																cadmium::example::gpt::GenrIntrHandler
																>
															>(model, clock);
			// auto rootCoordinator = cadmium::RootCoordinator(model);
		#endif

		#ifndef NO_LOGGING
		rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
		// rootCoordinator.setLogger<cadmium::CSVLogger>("trafficLightLog.csv", ";");
		#endif

		rootCoordinator.start();

		#ifdef RT_ESP32
		rootCoordinator.simulate(std::numeric_limits<double>::infinity());
		#else
		rootCoordinator.simulate(23.0);
		#endif

		rootCoordinator.stop();	

		#ifndef RT_ESP32
						return 0;
		#endif
	}
}

