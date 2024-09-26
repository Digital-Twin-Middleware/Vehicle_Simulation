#include <string>
#include <iostream>

#include "mqtt_client.h"

using namespace std;

namespace digital_twin {
    void Mqtt_client::test(string name) {
        cout << "Hello " << name << "!" << endl;
    }
}


