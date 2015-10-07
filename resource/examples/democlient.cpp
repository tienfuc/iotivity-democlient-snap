//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// OCClient.cpp : Defines the entry point for the console application.
//
#include <string>
#include <map>
#include <cstdlib>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

typedef std::map<OCResourceIdentifier, std::shared_ptr<OCResource>> DiscoveredResourceMap;

DiscoveredResourceMap discoveredResources;
std::shared_ptr<OCResource> sensorResource;
std::shared_ptr<OCResource> ledResource;
static ObserveType OBSERVE_TYPE_TO_USE = ObserveType::Observe;
std::mutex curResourceLock;

class Demo
{
public:

	double sensor_temp;
	double sensor_humidity;
	int sensor_light;
	bool led_red;
	bool led_green;
	bool led_blue;

	std::string m_name;

	Demo() : sensor_temp(0.0), sensor_humidity(0.0), sensor_light(0), 
		led_red(false), m_name("")
	{
	}
};

Demo mydemo;

int observe_count()
{
	static int oc = 0;
	return ++oc;
}

void onObserve(const HeaderOptions /*headerOptions*/, const OCRepresentation& rep,
                    const int& eCode, const int& sequenceNumber)
{
	try
	{
		if(eCode == OC_STACK_OK && sequenceNumber != OC_OBSERVE_NO_OPTION)
		{
			if(sequenceNumber == OC_OBSERVE_REGISTER)
			{
				std::cout << "Observe registration action is successful" << std::endl;
			}
			else if(sequenceNumber == OC_OBSERVE_DEREGISTER)
			{
				std::cout << "Observe De-registration action is successful" << std::endl;
			}

#if 0
			std::cout << "OBSERVE RESULT:"<<std::endl;
			std::cout << "\tSequenceNumber: "<< sequenceNumber << std::endl;
			rep.getValue("temperature", mydemo.m_temp);
			rep.getValue("humidity", mydemo.m_humidity);
			rep.getValue("name", mydemo.m_name);

			std::cout << "\ttemperature: " << mydemo.m_temp << std::endl;
			std::cout << "\thumidity: " << mydemo.m_humidity << std::endl;
			std::cout << "\tname: " << mydemo.m_name << std::endl;
#endif
			if(observe_count() == 11)
			{
				std::cout<<"Cancelling Observe..."<<std::endl;
				//OCStackResult result = curResource->cancelObserve();

				//std::cout << "Cancel result: "<< result <<std::endl;
				sleep(10);
				std::cout << "DONE"<<std::endl;
				std::exit(0);
			}
		}
		else
		{
			if(sequenceNumber == OC_OBSERVE_NO_OPTION)
			{
				std::cout << "Observe registration or de-registration action is failed" 
					<< std::endl;
			}
			else
			{
				std::cout << "onObserve Response error: " << eCode << std::endl;
				std::exit(-1);
			}
		}
	}
	catch(std::exception& e)
	{
		std::cout << "Exception: " << e.what() << " in onObserve" << std::endl;
	}

}

void onPost2(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED)
        {
            std::cout << "POST request was successful" << std::endl;

            if(rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                    << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
#if 0
                rep.getValue("temperature", mydemo.m_temp);
                rep.getValue("humidity", mydemo.m_humidity);
                rep.getValue("name", mydemo.m_name);

                std::cout << "\ttemperature: " << mydemo.m_temp << std::endl;
                std::cout << "\thumidity: " << mydemo.m_humidity << std::endl;
                std::cout << "\tname: " << mydemo.m_name << std::endl;
#endif
            }

            if (OBSERVE_TYPE_TO_USE == ObserveType::Observe)
                std::cout << std::endl << "Observe is used." << std::endl << std::endl;
            else if (OBSERVE_TYPE_TO_USE == ObserveType::ObserveAll)
                std::cout << std::endl << "ObserveAll is used." << std::endl << std::endl;

            //curResource->observe(OBSERVE_TYPE_TO_USE, QueryParamsMap(), &onObserve);

        }
        else
        {
            std::cout << "onPost2 Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost2" << std::endl;
    }

}

void onPost(const HeaderOptions& /*headerOptions*/,
        const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK || eCode == OC_STACK_RESOURCE_CREATED)
        {
            std::cout << "POST request was successful" << std::endl;

            if(rep.hasAttribute("createduri"))
            {
                std::cout << "\tUri of the created resource: "
                    << rep.getValue<std::string>("createduri") << std::endl;
            }
            else
            {
#if 0
                rep.getValue("temperature", mydemo.m_temp);
                rep.getValue("humidity", mydemo.m_humidity);
                rep.getValue("name", mydemo.m_name);

                std::cout << "\ttemperature: " << mydemo.m_temp << std::endl;
                std::cout << "\thumidity: " << mydemo.m_humidity << std::endl;
                std::cout << "\tname: " << mydemo.m_name << std::endl;
#endif
            }

            OCRepresentation rep2;

            std::cout << "Posting light representation..."<<std::endl;

#if 0
            mydemo.m_temp = 1; 
            mydemo.m_humidity = 2;

            rep2.setValue("temperature", mydemo.m_temp);
            rep2.setValue("humidity", mydemo.m_humidity);
            curResource->post(rep2, QueryParamsMap(), &onPost2);
#endif
        }
        else
        {
            std::cout << "onPost Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPost" << std::endl;
    }
}

// Local function to put a different state for this resource
void postDemoRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Posting light representation..."<<std::endl;

#if 0
        mydemo.m_temp = 5;
        mydemo.m_humidity = 6;

        rep.setValue("temperature", mydemo.m_temp);
        rep.setValue("humidity", mydemo.m_humidity);
#endif
        // Invoke resource's post API with rep, query map and the callback parameter
        resource->post(rep, QueryParamsMap(), &onPost);
    }
}

// callback handler on PUT request
void onPut(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "PUT request was successful" << std::endl;

            rep.getValue("temperature", mydemo.sensor_temp);
            rep.getValue("humidity", mydemo.sensor_humidity);
            rep.getValue("light", mydemo.sensor_light);

            std::cout << "\ttemperature: " << mydemo.sensor_humidity << std::endl;
            std::cout << "\thumidity: " << mydemo.sensor_humidity << std::endl;
            std::cout << "\tlight: " << mydemo.sensor_light << std::endl;

            postDemoRepresentation(sensorResource);
        }
        else
        {
            std::cout << "onPut Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onPut" << std::endl;
    }
}

// Local function to put a different state for this resource
void putSensorRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Putting sensor representation..."<<std::endl;

        mydemo.sensor_temp = 20;
        mydemo.sensor_humidity = 40;
        mydemo.sensor_light = 50;

        rep.setValue("temperature", mydemo.sensor_temp);
        rep.setValue("humidity", mydemo.sensor_humidity);
        rep.setValue("light", mydemo.sensor_light);

        // Invoke resource's put API with rep, query map and the callback parameter
        resource->put(rep, QueryParamsMap(), &onPut);
    }
}

void putLedRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        OCRepresentation rep;

        std::cout << "Putting LED representation..."<<std::endl;

        mydemo.led_red = true;
        mydemo.led_green = true;
        mydemo.led_blue = true;

        rep.setValue("red", mydemo.led_red);
        rep.setValue("green", mydemo.led_green);
        rep.setValue("blue", mydemo.led_blue);

        // Invoke resource's put API with rep, query map and the callback parameter
        resource->put(rep, QueryParamsMap(), &onPut);
    }
}

// Callback handler on GET request
void onGetSensor(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("temperature", mydemo.sensor_temp);
            rep.getValue("humidity", mydemo.sensor_humidity);
            rep.getValue("light", mydemo.sensor_light);
            rep.getValue("name", mydemo.m_name);

            std::cout << "\ttemperature: " << mydemo.sensor_temp << std::endl;
            std::cout << "\thumidity: " << mydemo.sensor_humidity << std::endl;
            std::cout << "\tlight: " << mydemo.sensor_light << std::endl;
            std::cout << "\tname: " << mydemo.m_name << std::endl;

            //putSensorRepresentation(sensorResource);
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGetSensor" << std::endl;
    }
}

void onGetLed(const HeaderOptions& /*headerOptions*/, const OCRepresentation& rep, const int eCode)
{
    try
    {
        if(eCode == OC_STACK_OK)
        {
            std::cout << "GET request was successful" << std::endl;
            std::cout << "Resource URI: " << rep.getUri() << std::endl;

            rep.getValue("red", mydemo.led_red);
            rep.getValue("green", mydemo.led_green);
            rep.getValue("blue", mydemo.led_blue);
            rep.getValue("name", mydemo.m_name);

            std::cout << "\tred: " << mydemo.led_red << std::endl;
            std::cout << "\tgreen: " << mydemo.led_green << std::endl;
            std::cout << "\tblue: " << mydemo.led_blue << std::endl;
            std::cout << "\tname: " << mydemo.m_name << std::endl;

            //putLedRepresentation(ledResource);
        }
        else
        {
            std::cout << "onGET Response error: " << eCode << std::endl;
            std::exit(-1);
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Exception: " << e.what() << " in onGetLed" << std::endl;
    }
}

// Local function to get representation of light resource
void getSensorRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        std::cout << "Getting Sensor Representation..."<<std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        resource->get(test, &onGetSensor);
    }
}

void getLedRepresentation(std::shared_ptr<OCResource> resource)
{
    if(resource)
    {
        std::cout << "Getting Representation..."<<std::endl;
        // Invoke resource's get API with the callback parameter

        QueryParamsMap test;
        resource->get(test, &onGetSensor);
    }
}

// Callback to found resources
void foundResource(std::shared_ptr<OCResource> resource)
{
    std::cout << "In foundResource\n";
    std::string resourceURI;
    std::string hostAddress;
    try
    {
        {
            std::lock_guard<std::mutex> lock(curResourceLock);
            if(discoveredResources.find(resource->uniqueIdentifier()) == discoveredResources.end())
            {
                std::cout << "Found resource " << resource->uniqueIdentifier() <<
                    " for the first time on server with ID: "<< resource->sid()<<std::endl;
                discoveredResources[resource->uniqueIdentifier()] = resource;
            }
            else
            {
                std::cout<<"Found resource "<< resource->uniqueIdentifier() << " again!"<<std::endl;
            }

            if(sensorResource && ledResource)
            {
                std::cout << "Found another resource, ignoring"<<std::endl;
                return;
            }
        }

        // Do some operations with resource object.
        if(resource)
        {
            std::cout<<"DISCOVERED Resource:"<<std::endl;
            // Get the resource URI
            resourceURI = resource->uri();
            std::cout << "\tURI of the resource: " << resourceURI << std::endl;

            // Get the resource host address
            hostAddress = resource->host();
            std::cout << "\tHost address of the resource: " << hostAddress << std::endl;

            // Get the resource types
            std::cout << "\tList of resource types: " << std::endl;
            for(auto &resourceTypes : resource->getResourceTypes())
            {
                std::cout << "\t\t" << resourceTypes << std::endl;
            }

            // Get the resource interfaces
            std::cout << "\tList of resource interfaces: " << std::endl;
            for(auto &resourceInterfaces : resource->getResourceInterfaces())
            {
                std::cout << "\t\t" << resourceInterfaces << std::endl;
            }

            if(resourceURI == "/grovepi/sensor")
            {
                sensorResource = resource;
                // Call a local function which will internally invoke get API on the resource pointer
                //getSensorRepresentation(resource);
            }
            if(resourceURI == "/grovepi/led")
            {
                ledResource = resource;
                // Call a local function which will internally invoke get API on the resource pointer
                //getLedRepresentation(resource);
            }
        }
        else
        {
            // Resource is invalid
            std::cout << "Resource is invalid" << std::endl;
        }

    }
    catch(std::exception& e)
    {
        std::cerr << "Exception in foundResource: "<< e.what() << std::endl;
    }
}

void printUsage()
{
    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------\n";
    std::cout << "Usage : simpleclient <ObserveType>" << std::endl;
    std::cout << "   ObserveType : 1 - Observe" << std::endl;
    std::cout << "   ObserveType : 2 - ObserveAll" << std::endl;
    std::cout << "---------------------------------------------------------------------\n\n";
}

void checkObserverValue(int value)
{
    if (value == 1)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::Observe;
        std::cout << "<===Setting ObserveType to Observe===>\n\n";
    }
    else if (value == 2)
    {
        OBSERVE_TYPE_TO_USE = ObserveType::ObserveAll;
        std::cout << "<===Setting ObserveType to ObserveAll===>\n\n";
    }
    else
    {
        std::cout << "<===Invalid ObserveType selected."
                  <<" Setting ObserveType to Observe===>\n\n";
    }
}

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen("./oic_svr_db_client.json", mode);
}

static void print_menu()
{
	std::cout << "Demo client menu" << std::endl;
	std::cout << "1 : read sensors" << std::endl;
	std::cout << "2 : control leds" << std::endl;
}

static void print_menu_sensor()
{
	std::cout << "1 : read temperature sensor" << std::endl;
	std::cout << "2 : read humidity sensor" << std::endl;
	std::cout << "3 : read light sensor" << std::endl;
}

static int sensor_read(int sensor)
{
	getSensorRepresentation(sensorResource);
}

static void print_menu_led()
{
	std::cout << "1 : turn on red LED" << std::endl;
	std::cout << "2 : turn on green LED" << std::endl;
	std::cout << "3 : turn on blue LED" << std::endl;
	std::cout << "4 : turn off red LED" << std::endl;
	std::cout << "5 : turn off green LED" << std::endl;
	std::cout << "6 : turn off blue LED" << std::endl;
}

static int led_read(int led)
{

}

static int led_write(int led)
{

}

int main(int argc, char* argv[]) {

	std::ostringstream requestURI;
	OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

#if 0
    try
    {
        printUsage();
        if (argc == 1)
        {
            std::cout << "<===Setting ObserveType to Observe and ConnectivityType to IP===>\n\n";
        }
        else if (argc == 2)
        {
            checkObserverValue(std::stoi(argv[1]));
        }
        else
        {
            std::cout << "<===Invalid number of command line arguments===>\n\n";
            return -1;
        }
    }
    catch(std::exception& )
    {
        std::cout << "<===Invalid input arguments===>\n\n";
        return -1;
    }
#endif

	std::cout << "Configuring ... " << std::endl;
	// Create PlatformConfig object
	PlatformConfig cfg {
		OC::ServiceType::InProc,
		OC::ModeType::Both,
		"0.0.0.0",
		0,
		OC::QualityOfService::LowQos,
		&ps
	};

	OCPlatform::Configure(cfg);
	try
	{
		// makes it so that all boolean values are printed as 'true/false' in this stream
		std::cout.setf(std::ios::boolalpha);
		// Find all resources
		//requestURI << OC_RSRVD_WELL_KNOWN_URI;// << "?rt=core.light";
		requestURI << OC_RSRVD_WELL_KNOWN_URI << "?rt=grovepi.sensor";// << "?rt=core.light";

		OCPlatform::findResource("", requestURI.str(), CT_DEFAULT, &foundResource);
		std::cout<< "Finding Resource... " <<std::endl;

#if 0
        // Find resource is done twice so that we discover the original resources a second time.
        // These resources will have the same uniqueidentifier (yet be different objects), so that
        // we can verify/show the duplicate-checking code in foundResource(above);
        OCPlatform::findResource("", requestURI.str(),
                CT_DEFAULT, &foundResource);
        std::cout<< "Finding Resource for second time..." << std::endl;
#endif
#if 0
        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        cv.wait(lock);
#else
	while(true)
	{
		int cmd, cmd1;
		print_menu();
		std::cin >> cmd;
		switch(cmd)
		{
			case 1:
				print_menu_sensor();
				std::cin >> cmd1;
				if(cmd1 >= 1 && cmd1 <= 3)
					sensor_read(cmd1);
				else
					std::cout << "Unknown option: " << cmd1 << std::endl;
				break;
			case 2:
				print_menu_led();
				std::cin >> cmd1;
				if(cmd1 >= 1 && cmd1 <= 3)
					led_read(cmd1);
				else if(cmd1 >= 4 && cmd1 <= 6)
					led_write(cmd1);
				else
					std::cout << "Unknown option: " << cmd1 << std::endl;
				break;
			default:
				std::cout << "Unknown option: " << cmd << std::endl;
		}
	}
#endif
	}
	catch(OCException& e)
	{
		oclog() << "Exception in main: "<<e.what();
	}

	return 0;
}


