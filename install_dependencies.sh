cd ~
git clone https://github.com/eclipse/paho.mqtt.cpp
cd paho.mqtt.cpp
git submodule init
git submodule update
cmake -Bbuild -H. -DPAHO_WITH_MQTT_C=ON -DPAHO_BUILD_EXAMPLES=ON
sudo cmake --build build/ --target install