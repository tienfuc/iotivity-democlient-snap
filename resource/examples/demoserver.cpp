#include <Python.h>


#include <functional>

#include <pthread.h>
#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;
using namespace std;
namespace PH = std::placeholders;

int gObservation = 0;
void * ChangeDemoRepresentation (void *param);
void * handleSlowResponse (void *param, std::shared_ptr<OCResourceRequest> pRequest);

// Specifies where to notify all observers or list of observers
// false: notifies all observers
// true: notifies list of observers
bool isListOfObservers = false;

// Specifies secure or non-secure
// false: non-secure resource
// true: secure resource
bool isSecure = false;

/// Specifies whether Entity handler is going to do slow response or not
bool isSlowResponse = false;

// Forward declaring the entityHandler

/// This class represents a single resource named 'lightResource'. This resource has
/// two simple properties named 'state' and 'power'

class DemoResource
{

public:
	std::string demo_name;

	std::string sensor_name;
	double sensor_temp;
	double sensor_humidity;
	int sensor_light;
	OCResourceHandle sensor_resourceHandle;
	OCRepresentation sensor_rep;

	bool led_red;
	bool led_green;
	bool led_blue;
	OCResourceHandle led_resourceHandle;
	OCRepresentation led_rep;

	ObservationIds m_interestedObservers;

	std::string py_path = "../../../../../../extlibs/GrovePi/Software/Python";

public:
	DemoResource()
		:demo_name("Demo resource"), 
		sensor_name("Grovepi sensor"), sensor_temp(0.0), sensor_humidity(0.0), sensor_light(0),
		led_red(false), led_green(false), led_blue(false),
		sensor_resourceHandle(nullptr), led_resourceHandle(nullptr) 
	{
		// Initialize representation
		sensor_rep.setUri("/grovepi/sensor");
		sensor_rep.setValue("temperature", sensor_temp);
		sensor_rep.setValue("humidity", sensor_humidity);
		sensor_rep.setValue("light", sensor_light);
		sensor_rep.setValue("name", sensor_name);

		led_rep.setUri("/grovepi/led");
		led_rep.setValue("red", led_red);
		led_rep.setValue("green", led_green);
		led_rep.setValue("blue", led_blue);
		led_rep.setValue("name", "Grovepi LED");

		setenv("PYTHONPATH", py_path.c_str(), 1);
	}


	/* Note that this does not need to be a member function: for classes you do not have
	access to, you can accomplish this with a free function: */

	/// This function internally calls registerResource API.
	void createResource()
	{
		//URI of the resource
		std::string resourceURI = "/grovepi/sensor";
		//resource type name. In this case, it is light
		std::string resourceTypeName = "grovepi.sensor";
		// resource interface.
		std::string resourceInterface = DEFAULT_INTERFACE;

		// OCResourceProperty is defined ocstack.h
		uint8_t resourceProperty;
		if(isSecure)
		{
			resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE;
		}
		else
		{
			resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;
		}
		EntityHandler sensor_cb = std::bind(&DemoResource::sensor_entityHandler, this,PH::_1);
		EntityHandler led_cb = std::bind(&DemoResource::led_entityHandler, this,PH::_1);

		// This will internally create and register the resource.
		OCStackResult result = OCPlatform::registerResource(
			sensor_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, sensor_cb, resourceProperty);

		if (OC_STACK_OK != result)
		{
			cout << "Resource creation was unsuccessful\n";
		}

#if 0
		resourceURI = "/grovepi/led";
		resourceTypeName = "grovepi.led";
		result = OCPlatform::registerResource(
			led_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, led_cb, resourceProperty);

		if (OC_STACK_OK != result)
		{
			cout << "Resource creation was unsuccessful\n";
		}
#endif
	}

	PyObject* call_py_func(char *func_name)
	{
		PyObject *p_filename, *p_module, *p_dict, *p_func, *p_result = NULL;

		// Initialize the Python Interpreter
		Py_Initialize();
		// Build the name object
		p_filename = PyString_FromString((char *)"grovepilib");
		// Load the module object
		p_module = PyImport_Import(p_filename);
		// pDict is a borrowed reference 
		p_dict = PyModule_GetDict(p_module);
	
		p_func = PyDict_GetItemString(p_dict, func_name);
		if(PyCallable_Check(p_func))
		{
			std::cout << "Access grovepi library" << std::endl;
			p_result = PyObject_CallObject(p_func, NULL);
			PyErr_Print();
		}
		else
		{
			std::cout << "Can not access Grovepi library" << std::endl;
			PyErr_Print();
		}

		Py_DECREF(p_filename);
		Py_DECREF(p_module);
		Py_DECREF(p_dict);
		// Finish the Python Interpreter
		Py_Finalize();

		return p_result;
	}

	double sensor_read_temp()
	{
		PyObject *p_result;

		p_result = call_py_func("sensor_read_temp");

		if(p_result)
		{
			//std::cout << PyFloat_AsDouble(p_result) << std::endl;
			return PyFloat_AsDouble(p_result);
		}
		else
		{
			return -1;
		}
	}

	double sensor_read_humidity()
	{
		PyObject *p_result;

		p_result = call_py_func("sensor_read_humidity");

		if(p_result)
		{
			//std::cout << PyFloat_AsDouble(p_result) << std::endl;
			return PyFloat_AsDouble(p_result);
		}
		else
		{
			return -1;
		}
	}

	int sensor_read_light()
	{
		PyObject *p_result;

		p_result = call_py_func("sensor_read_light");

		if(p_result)
		{
			//std::cout << PyInt_AsLong(p_result) << std::endl;
			return PyInt_AsLong(p_result);
		}
		else
		{
			return -1;
		}
	}


	OCResourceHandle getHandle()
	{
		return sensor_resourceHandle;
	}

	// Puts representation.
	// Gets values from the representation and
	// updates the internal state
	void put(OCRepresentation& rep)
	{
		try {
			if (rep.getValue("temperature", sensor_temp))
			{
				cout << "\t\t\t\t" << "temperature: " << sensor_temp << endl;
			}
			else
			{
				cout << "\t\t\t\t" << "state not found in the representation" << endl;
			}

			if (rep.getValue("humidity", sensor_humidity))
			{
				cout << "\t\t\t\t" << "humidity: " << sensor_humidity << endl;
			}
			else
			{
				cout << "\t\t\t\t" << "power not found in the representation" << endl;
			}

			if (rep.getValue("light", sensor_light))
			{
				cout << "\t\t\t\t" << "light: " << sensor_light << endl;
			}
			else
			{
				cout << "\t\t\t\t" << "power not found in the representation" << endl;
			}
		}
		catch (exception& e)
		{
			cout << e.what() << endl;
		}
	}

    // Post representation.
    // Post can create new resource or simply act like put.
    // Gets values from the representation and
    // updates the internal state
    OCRepresentation post(OCRepresentation& rep)
    {
#if 0
        static int first = 1;

        // for the first time it tries to create a resource
        if(first)
        {
            first = 0;

            if(OC_STACK_OK == createResource1())
            {
                OCRepresentation rep1;
                rep1.setValue("createduri", std::string("/a/light1"));

                return rep1;
            }
        }

        // from second time onwards it just puts
        put(rep);
#endif
        return get();
    }


    // gets the updated representation.
    // Updates the representation with latest internal state before
    // sending out.
    OCRepresentation get()
    {
	sensor_temp = sensor_read_temp();
	sensor_humidity = sensor_read_humidity();
	sensor_light = sensor_read_light();

        sensor_rep.setValue("temperature", sensor_temp);
        sensor_rep.setValue("humidity", sensor_humidity);
        sensor_rep.setValue("light", sensor_light);
        return sensor_rep;
    }

    void addType(const std::string& type) const
    {
        OCStackResult result = OCPlatform::bindTypeToResource(sensor_resourceHandle, type);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

    void addInterface(const std::string& interface) const
    {
        OCStackResult result = OCPlatform::bindInterfaceToResource(sensor_resourceHandle, interface);
        if (OC_STACK_OK != result)
        {
            cout << "Binding TypeName to Resource was unsuccessful\n";
        }
    }

private:
// This is just a sample implementation of entity handler.
// Entity handler can be implemented in several ways by the manufacturer
OCEntityHandlerResult sensor_entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server sensor entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            // Check for query params (if any)
            QueryParamsMap queries = request->getQueryParameters();

            if (!queries.empty())
            {
                std::cout << "\nQuery processing upto entityHandler" << std::endl;
            }
            for (auto it : queries)
            {
                std::cout << "Query key: " << it.first << " value : " << it.second
                        << std:: endl;
            }

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
#if 0
                if(isSlowResponse) // Slow response case
                {
                    static int startedThread = 0;
                    if(!startedThread)
                    {
                        std::thread t(handleSlowResponse, (void *)this, request);
                        startedThread = 1;
                        t.detach();
                    }
                    ehResult = OC_EH_SLOW;
                }
                else // normal response case.
#endif
                {
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(get());
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                // Update the lightResource
                put(rep);
                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(get());
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
            }
            else if(requestType == "POST")
            {
                cout << "\t\t\trequestType : POST\n";
#if 0
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to POST request
                OCRepresentation rep_post = post(rep);
                pResponse->setResourceRepresentation(rep_post);
                pResponse->setErrorCode(200);
                if(rep_post.hasAttribute("createduri"))
                {
                    pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                    pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
                }
                else
                {
                    pResponse->setResponseResult(OC_EH_OK);
                }

                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
#endif
            }
            else if(requestType == "DELETE")
            {
                cout << "Delete request received" << endl;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = request->getObservationInfo();
            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_interestedObservers.erase(std::remove(
                                                            m_interestedObservers.begin(),
                                                            m_interestedObservers.end(),
                                                            observationInfo.obsId),
                                                            m_interestedObservers.end());
            }

            pthread_t threadId;

            cout << "\t\trequestFlag : Observer\n";
#if 0
            gObservation = 1;
            static int startedThread = 0;

            // Observation happens on a different thread in ChangeLightRepresentation function.
            // If we have not created the thread already, we will create one here.
            if(!startedThread)
            {
                pthread_create (&threadId, NULL, ChangeDemoRepresentation, (void *)this);
                startedThread = 1;
            }
#endif
            ehResult = OC_EH_OK;
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return ehResult;
}

OCEntityHandlerResult led_entityHandler(std::shared_ptr<OCResourceRequest> request)
{
    cout << "\tIn Server CPP entity handler:\n";
    OCEntityHandlerResult ehResult = OC_EH_ERROR;
    if(request)
    {
        // Get the request type and request flag
        std::string requestType = request->getRequestType();
        int requestFlag = request->getRequestHandlerFlag();

        if(requestFlag & RequestHandlerFlag::RequestFlag)
        {
            cout << "\t\trequestFlag : Request\n";
            auto pResponse = std::make_shared<OC::OCResourceResponse>();
            pResponse->setRequestHandle(request->getRequestHandle());
            pResponse->setResourceHandle(request->getResourceHandle());

            // Check for query params (if any)
            QueryParamsMap queries = request->getQueryParameters();

            if (!queries.empty())
            {
                std::cout << "\nQuery processing upto entityHandler" << std::endl;
            }
            for (auto it : queries)
            {
                std::cout << "Query key: " << it.first << " value : " << it.second
                        << std:: endl;
            }

            // If the request type is GET
            if(requestType == "GET")
            {
                cout << "\t\t\trequestType : GET\n";
#if 0
                if(isSlowResponse) // Slow response case
                {
                    static int startedThread = 0;
                    if(!startedThread)
                    {
                        std::thread t(handleSlowResponse, (void *)this, request);
                        startedThread = 1;
                        t.detach();
                    }
                    ehResult = OC_EH_SLOW;
                }
                else // normal response case.
                {
                    pResponse->setErrorCode(200);
                    pResponse->setResponseResult(OC_EH_OK);
                    pResponse->setResourceRepresentation(get());
                    if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                    {
                        ehResult = OC_EH_OK;
                    }
                }
#endif
            }
            else if(requestType == "PUT")
            {
                cout << "\t\t\trequestType : PUT\n";
#if 0
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to PUT request
                // Update the lightResource
                put(rep);
                pResponse->setErrorCode(200);
                pResponse->setResponseResult(OC_EH_OK);
                pResponse->setResourceRepresentation(get());
                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
#endif
            }
            else if(requestType == "POST")
            {
                cout << "\t\t\trequestType : POST\n";
#if 0
                OCRepresentation rep = request->getResourceRepresentation();

                // Do related operations related to POST request
                OCRepresentation rep_post = post(rep);
                pResponse->setResourceRepresentation(rep_post);
                pResponse->setErrorCode(200);
                if(rep_post.hasAttribute("createduri"))
                {
                    pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
                    pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
                }
                else
                {
                    pResponse->setResponseResult(OC_EH_OK);
                }

                if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
                {
                    ehResult = OC_EH_OK;
                }
#endif
            }
            else if(requestType == "DELETE")
            {
                cout << "Delete request received" << endl;
            }
        }

        if(requestFlag & RequestHandlerFlag::ObserverFlag)
        {
            ObservationInfo observationInfo = request->getObservationInfo();
            if(ObserveAction::ObserveRegister == observationInfo.action)
            {
                m_interestedObservers.push_back(observationInfo.obsId);
            }
            else if(ObserveAction::ObserveUnregister == observationInfo.action)
            {
                m_interestedObservers.erase(std::remove(
                                                            m_interestedObservers.begin(),
                                                            m_interestedObservers.end(),
                                                            observationInfo.obsId),
                                                            m_interestedObservers.end());
            }

            pthread_t threadId;

            cout << "\t\trequestFlag : Observer\n";
#if 0
            gObservation = 1;
            static int startedThread = 0;

            // Observation happens on a different thread in ChangeLightRepresentation function.
            // If we have not created the thread already, we will create one here.
            if(!startedThread)
            {
                pthread_create (&threadId, NULL, ChangeDemoRepresentation, (void *)this);
                startedThread = 1;
            }
#endif
            ehResult = OC_EH_OK;
        }
    }
    else
    {
        std::cout << "Request invalid" << std::endl;
    }

    return ehResult;
}

};


class PyGrovepi
{
	public:
};


// ChangeLightRepresentaion is an observation function,
// which notifies any changes to the resource to stack
// via notifyObservers
void * ChangeDemoRepresentation (void *param)
{
    DemoResource* demoPtr = (DemoResource*) param;

#if 0
    // This function continuously monitors for the changes
    while (1)
    {
        sleep (3);

        if (gObservation)
        {
            // If under observation if there are any changes to the light resource
            // we call notifyObservors
            //
            // For demostration we are changing the power value and notifying.
            demoPtr->m_humidity += 10;

            cout << "\nHumidity updated to : " << demoPtr->m_humidity << endl;
            cout << "Notifying observers with resource handle: " << demoPtr->getHandle() << endl;

            OCStackResult result = OC_STACK_OK;

            if(isListOfObservers)
            {
                std::shared_ptr<OCResourceResponse> resourceResponse =
                            {std::make_shared<OCResourceResponse>()};

                resourceResponse->setErrorCode(200);
                resourceResponse->setResourceRepresentation(demoPtr->get(), DEFAULT_INTERFACE);

                result = OCPlatform::notifyListOfObservers(  demoPtr->getHandle(),
                                                             demoPtr->m_interestedObservers,
                                                             resourceResponse);
            }
            else
            {
                result = OCPlatform::notifyAllObservers(demoPtr->getHandle());
            }

            if(OC_STACK_NO_OBSERVERS == result)
            {
                cout << "No More observers, stopping notifications" << endl;
                gObservation = 0;
            }
        }
    }
#endif
    return NULL;
}

void * handleSlowResponse (void *param, std::shared_ptr<OCResourceRequest> pRequest)
{
    // This function handles slow response case
    DemoResource* demoPtr = (DemoResource*) param;
    // Induce a case for slow response by using sleep
    std::cout << "SLOW response" << std::endl;
    sleep (10);

    auto pResponse = std::make_shared<OC::OCResourceResponse>();
    pResponse->setRequestHandle(pRequest->getRequestHandle());
    pResponse->setResourceHandle(pRequest->getResourceHandle());
    pResponse->setResourceRepresentation(demoPtr->get());
    pResponse->setErrorCode(200);
    pResponse->setResponseResult(OC_EH_OK);

    // Set the slow response flag back to false
    isSlowResponse = false;
    OCPlatform::sendResponse(pResponse);

    return NULL;
}

void PrintUsage()
{
    std::cout << std::endl;
    std::cout << "Usage : simpleserver <value>\n";
    std::cout << "    Default - Non-secure resource and notify all observers\n";
    std::cout << "    1 - Non-secure resource and notify list of observers\n\n";
    std::cout << "    2 - Secure resource and notify all observers\n";
    std::cout << "    3 - Secure resource and notify list of observers\n\n";
    std::cout << "    4 - Non-secure resource, GET slow response, notify all observers\n";
}

static FILE* client_open(const char* /*path*/, const char *mode)
{
    return fopen("./oic_svr_db_server.json", mode);
}

int main(int argc, char* argv[])
{
    PrintUsage();
    OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

    if (argc == 1)
    {
        isListOfObservers = false;
        isSecure = false;
    }
    else if (argc == 2)
    {
        int value = atoi(argv[1]);
        switch (value)
        {
            case 1:
                isListOfObservers = true;
                isSecure = false;
                break;
            case 2:
                isListOfObservers = false;
                isSecure = true;
                break;
            case 3:
                isListOfObservers = true;
                isSecure = true;
                break;
            case 4:
                isSlowResponse = true;
                break;
            default:
                break;
       }
     }
    else
    {
        return -1;
    }

    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos,
        &ps
    };

    OCPlatform::Configure(cfg);
    try
    {
        // Create the instance of the resource class
        // (in this case instance of class 'LightResource').
        DemoResource myDemo;

        // Invoke createResource function of class light.
        myDemo.createResource();
        std::cout << "Created resource." << std::endl;

	//myDemo.addType(std::string("demo.grovepi"));
        //myDemo.addInterface(std::string(LINK_INTERFACE));
        //std::cout << "Added Interface and Type" << std::endl;


        // A condition variable will free the mutex it is given, then do a non-
        // intensive block until 'notify' is called on it.  In this case, since we
        // don't ever call cv.notify, this should be a non-processor intensive version
        // of while(true);
        std::mutex blocker;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(blocker);
        std::cout <<"Waiting" << std::endl;
        cv.wait(lock, []{return false;});
    }
    catch(OCException &e)
    {
        std::cout << "OCException in main : " << e.what() << endl;
    }

    // No explicit call to stop the platform.
    // When OCPlatform::destructor is invoked, internally we do platform cleanup

    return 0;
}
