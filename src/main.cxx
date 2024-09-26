#include <iostream>
#include "digital_twin_client.h"

using namespace digital_twin;

int main() {
    digital_twin_client client;

    client.connect();
	
	//client.disconnect();
	
    return 0;
}



