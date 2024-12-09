#include "cadmium/simulation/rt_root_coordinator.hpp"
#include "include/top.hpp"
#include "include/MQTT_interrupt_handler.hpp"
#include "cadmium/simulation/rt_clock/chrono.hpp"

#ifndef NO_LOGGING
	#include "cadmium/simulation/logger/stdout.hpp"
	#include "cadmium/simulation/logger/csv.hpp"
#endif

using namespace cadmium::example::gpt;

int main() {

	auto model = std::make_shared<top> ("top");

	cadmium::ChronoClock<std::chrono::steady_clock, int, cadmium::example::gpt::MQTTIntrHandler> clock(model);
	auto rootCoordinator = cadmium::RealTimeRootCoordinator<
															cadmium::ChronoClock<
																std::chrono::steady_clock, 
																int, 
																cadmium::example::gpt::MQTTIntrHandler
																>
															> (model, clock);

	#ifndef NO_LOGGING
	rootCoordinator.setLogger<cadmium::STDOUTLogger>(";");
	#endif

	rootCoordinator.start();

	rootCoordinator.simulate(std::numeric_limits<double>::infinity());

	rootCoordinator.stop();	

	return 0;
}

