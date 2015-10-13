#include <Python.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

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
void * CheckButtonRepresentation (void *param);
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

mutex py_func_lock;

class DemoResource
{

public:
	std::string demo_name;

	double sensor_temp;
	double sensor_humidity;
	int sensor_light;
	int sensor_sound;
	OCResourceHandle sensor_resourceHandle;
	OCRepresentation sensor_rep;

	int led_red;
	int led_green;
	int led_blue;
	OCResourceHandle led_resourceHandle;
	OCRepresentation led_rep;

	std::string lcd_str;
	OCResourceHandle lcd_resourceHandle;
	OCRepresentation lcd_rep;

	double buzzer;
	OCResourceHandle buzzer_resourceHandle;
	OCRepresentation buzzer_rep;

	int button;
	OCResourceHandle button_resourceHandle;
	OCRepresentation button_rep;

	ObservationIds m_interestedObservers;

	//std::string py_path = "/home/u/demo/iotivity/extlibs/GrovePi/Software/Python";

public:
	DemoResource()
		:demo_name("Demo resource"), 
		sensor_temp(0.0), sensor_humidity(0.0), sensor_light(0), sensor_sound(0),
		sensor_resourceHandle(nullptr),
		led_red(0), led_green(0), led_blue(0),
		led_resourceHandle(nullptr),
		lcd_str("LCD Demo"),
		lcd_resourceHandle(nullptr), 
		buzzer(0.0),
		buzzer_resourceHandle(nullptr),
		button(0),
	       	button_resourceHandle(nullptr)
	{
		// Initialize representation
		sensor_rep.setUri("/grovepi/sensor");
		sensor_rep.setValue("temperature", sensor_temp);
		sensor_rep.setValue("humidity", sensor_humidity);
		sensor_rep.setValue("light", sensor_light);
		sensor_rep.setValue("sound", sensor_sound);

		led_rep.setUri("/grovepi/led");
		led_rep.setValue("red", led_red);
		led_rep.setValue("green", led_green);
		led_rep.setValue("blue", led_blue);

		led_rep.setUri("/grovepi/lcd");
		led_rep.setValue("lcd", lcd_str);

		led_rep.setUri("/grovepi/buzzer");
		led_rep.setValue("buzzer", buzzer);

		led_rep.setUri("/grovepi/button");
		led_rep.setValue("button", button);

		//setenv("PYTHONPATH", py_path.c_str(), 1);
	}

	/* Note that this does not need to be a member function: for classes you do not have
	access to, you can accomplish this with a free function: */

	/// This function internally calls registerResource API.
	void createResource()
	{
		// Sensor resource
		std::string resourceURI = "/grovepi/sensor";
		std::string resourceTypeName = "grovepi.sensor";
		std::string resourceInterface = DEFAULT_INTERFACE;

		// OCResourceProperty is defined ocstack.h
		uint8_t resourceProperty;
		if(isSecure)
			resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE | OC_SECURE;
		else
			resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

		// Resource callback functions
		EntityHandler sensor_cb = std::bind(&DemoResource::sensor_entityHandler, this,PH::_1);
		EntityHandler led_cb = std::bind(&DemoResource::led_entityHandler, this,PH::_1);
		EntityHandler lcd_cb = std::bind(&DemoResource::lcd_entityHandler, this,PH::_1);
		EntityHandler buzzer_cb = std::bind(&DemoResource::buzzer_entityHandler, this,PH::_1);
		EntityHandler button_cb = std::bind(&DemoResource::button_entityHandler, this,PH::_1);

		// This will internally create and register the resource.
		OCStackResult result = OCPlatform::registerResource(
			sensor_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, sensor_cb, resourceProperty);

		if (OC_STACK_OK != result)
			cout << "Sensor resource creation was unsuccessful\n";

		// Create LED resource
		resourceURI = "/grovepi/led";
		resourceTypeName = "grovepi.led";
		result = OCPlatform::registerResource(
			led_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, led_cb, resourceProperty);

		if (OC_STACK_OK != result)
			cout << "LED resource creation was unsuccessful\n";

		// Create LCD resource
		resourceURI = "/grovepi/lcd";
		resourceTypeName = "grovepi.lcd";
		result = OCPlatform::registerResource(
			lcd_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, lcd_cb, resourceProperty);

		if (OC_STACK_OK != result)
			cout << "LCD resource creation was unsuccessful\n";

		// Create Buzzer resource
		resourceURI = "/grovepi/buzzer";
		resourceTypeName = "grovepi.buzzer";
		result = OCPlatform::registerResource(
			buzzer_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, buzzer_cb, resourceProperty);

		if (OC_STACK_OK != result)
			cout << "Buzzer resource creation was unsuccessful\n";

		// Create Button resource
		resourceURI = "/grovepi/button";
		resourceTypeName = "grovepi.button";
		result = OCPlatform::registerResource(
			button_resourceHandle, resourceURI, resourceTypeName,
			resourceInterface, button_cb, resourceProperty);

		if (OC_STACK_OK != result)
			cout << "Button resource creation was unsuccessful\n";
	}

	void py_prolog(PyObject **filename, PyObject **module, PyObject **dict, PyObject **func, char *func_name)
	{
		py_lock();

		// Initialize the Python Interpreter
		Py_Initialize();
		PyErr_Print();
		// Build the name object
		*filename = PyString_FromString((char *)"grovepilib");
		PyErr_Print();
		// Load the module object
		*module = PyImport_Import(*filename);
		PyErr_Print();
		// dict is a borrowed reference 
		*dict = PyModule_GetDict(*module);
		PyErr_Print();

		*func = PyDict_GetItemString(*dict, func_name);
		PyErr_Print();
	}

	void py_epilog(PyObject **filename, PyObject **module, PyObject **dict, PyObject **func)
	{
		Py_DECREF(*filename);
		Py_DECREF(*module);
		Py_DECREF(*dict);
		Py_DECREF(*func);
		// Finish the Python Interpreter
		Py_Finalize();

		py_unlock();
	}

	void py_lock()
	{
		py_func_lock.lock();
	}

	void py_unlock()
	{
		py_func_lock.unlock();
	}

	PyObject* py_func(char *func_name)
	{
		PyObject *p_filename, *p_module, *p_dict, *p_func, *p_result = NULL;

		py_prolog(&p_filename, &p_module, &p_dict, &p_func, func_name);
	
		if(PyCallable_Check(p_func)) {
			std::cout << "Access grovepi library" << std::endl;
			p_result = PyObject_CallObject(p_func, NULL);
			PyErr_Print();
		} else {
			std::cout << "Can not access Grovepi library" << std::endl;
			PyErr_Print();
		}

		py_epilog(&p_filename, &p_module, &p_dict, &p_func);

		return p_result;
	}

	PyObject* py_func(char *func_name, int value)
	{
		PyObject *p_filename, *p_module, *p_dict, *p_func, *p_value, *p_result = NULL;

		py_prolog(&p_filename, &p_module, &p_dict, &p_func, func_name);
	
		if(PyCallable_Check(p_func)) {
			std::cout << "Access grovepi library" << std::endl;
			p_value = Py_BuildValue("(i)", value);
			p_result = PyObject_CallObject(p_func, p_value);
			PyErr_Print();
		} else {
			std::cout << "Can not access Grovepi library" << std::endl;
			PyErr_Print();
		}

		py_epilog(&p_filename, &p_module, &p_dict, &p_func);

		return p_result;
	}

	PyObject* py_func(char *func_name, double value)
	{
		PyObject *p_filename, *p_module, *p_dict, *p_func, *p_value, *p_result = NULL;

		py_prolog(&p_filename, &p_module, &p_dict, &p_func, func_name);
	
		if(PyCallable_Check(p_func)) {
			std::cout << "Access grovepi library" << std::endl;
			p_value = Py_BuildValue("(d)", value);
			p_result = PyObject_CallObject(p_func, p_value);
			PyErr_Print();
		} else {
			std::cout << "Can not access Grovepi library" << std::endl;
			PyErr_Print();
		}

		py_epilog(&p_filename, &p_module, &p_dict, &p_func);

		return p_result;
	}

	PyObject* py_func(char *func_name, const char *str)
	{
		PyObject *p_filename, *p_module, *p_dict, *p_func, *p_value, *p_result = NULL;

		py_prolog(&p_filename, &p_module, &p_dict, &p_func, func_name);
	
		if(PyCallable_Check(p_func)) {
			std::cout << "Access grovepi library" << std::endl;
			p_value = Py_BuildValue("(s)", str);
			p_result = PyObject_CallObject(p_func, p_value);
			PyErr_Print();
		} else {
			std::cout << "Can not access Grovepi library" << std::endl;
			PyErr_Print();
		}

		py_epilog(&p_filename, &p_module, &p_dict, &p_func);

		return p_result;
	}

	double sensor_read_temp()
	{
		PyObject *p_result;
		p_result = py_func((char *)"sensor_read_temp");

		if(p_result)
			return PyFloat_AsDouble(p_result);
		else
			return -1;
	}

	double sensor_read_humidity()
	{
		PyObject *p_result;

		p_result = py_func((char *)"sensor_read_humidity");

		if(p_result)
			return PyFloat_AsDouble(p_result);
		else
			return -1;
	}

	int sensor_read_light()
	{
		PyObject *p_result;

		p_result = py_func((char *)"sensor_read_light");

		if(p_result)
			return PyInt_AsLong(p_result);
		else
			return -1;
	}

	int sensor_read_sound()
	{
		PyObject *p_result;

		p_result = py_func((char *)"sensor_read_sound");

		if(p_result)
			return PyInt_AsLong(p_result);
		else
			return -1;
	}

	int led_write_red(int status)
	{
		PyObject *p_result;

		p_result = py_func((char *)"led_write_red", status);

		if(p_result)
			return 0;
		else
			return -1;
	}

	int led_write_green(int status)
	{
		PyObject *p_result;

		p_result = py_func((char *)"led_write_green", status);

		if(p_result)
			return 0;
		else
			return -1;
	}

	int led_write_blue(int status)
	{
		PyObject *p_result;

		p_result = py_func((char *)"led_write_blue", status);

		if(p_result)
			return 0;
		else
			return -1;
	}

	int lcd_write_str(const char *str)
	{
		PyObject *p_result;

		p_result = py_func((char *)"lcd_write_str", str);

		if(p_result)
			return 0;
		else
			return -1;
	}

	int buzzer_write(double value)
	{
		PyObject *p_result;

		p_result = py_func((char *)"buzzer_write", value);

		if(p_result)
			return 0;
		else
			return -1;
	}

	int button_read()
	{
		PyObject *p_result;

		p_result = py_func((char *)"button_read");

		if(p_result)
			return PyInt_AsLong(p_result);
		else
			return -1;
	}


	// Puts representation.
	// Gets values from the representation and
	// updates the internal state
	void put_sensor(OCRepresentation& rep)
	{
		try {
			if (rep.getValue("temperature", sensor_temp))
				cout << "\t\t\t\t" << "temperature: " << sensor_temp << endl;
			else
				cout << "\t\t\t\t" << "temperature not found in the representation" << endl;

			if (rep.getValue("humidity", sensor_humidity))
				cout << "\t\t\t\t" << "humidity: " << sensor_humidity << endl;
			else
				cout << "\t\t\t\t" << "humidity not found in the representation" << endl;

			if (rep.getValue("light", sensor_light))
				cout << "\t\t\t\t" << "light: " << sensor_light << endl;
			else
				cout << "\t\t\t\t" << "light not found in the representation" << endl;

			if (rep.getValue("sound", sensor_sound))
				cout << "\t\t\t\t" << "sound: " << sensor_sound << endl;
			else
				cout << "\t\t\t\t" << "sound not found in the representation" << endl;
		} 
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void put_led(OCRepresentation& rep)
	{
		try {
			if (rep.getValue("red", led_red)) {
				cout << "\t\t\t\t" << "Red LED: " << led_red << endl;
				led_write_red(led_red);
			} else {
				cout << "\t\t\t\t" << "red not found in the representation" << endl;
			}

			if (rep.getValue("green", led_green)) {
				cout << "\t\t\t\t" << "Green LED: " << led_green << endl;
				led_write_green(led_green);
			} else {
				cout << "\t\t\t\t" << "green not found in the representation" << endl;
			}

			if (rep.getValue("blue", led_blue)) {
				cout << "\t\t\t\t" << "Blue LED: " << led_blue << endl;
				led_write_blue(led_blue);
			} else {
				cout << "\t\t\t\t" << "blue not found in the representation" << endl;
			}
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void put_lcd(OCRepresentation& rep)
	{
		try {
			if(rep.getValue("lcd", lcd_str)) {
				cout << "\t\t\t\t" << "LCD string: " << lcd_str << endl;
				lcd_write_str(lcd_str.c_str());
			} else {
				cout << "\t\t\t\t" << "LCD string not found in the representation" << endl;
			}
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void put_buzzer(OCRepresentation& rep)
	{
		try {
			if(rep.getValue("buzzer", buzzer)) {
				cout << "\t\t\t\t" << "Buzzer: " << buzzer << endl;
				buzzer_write(buzzer);
			} else {
				cout << "\t\t\t\t" << "buzzer not found in the representation" << endl;
			}
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

#if 0
	// Post representation.
	// Post can create new resource or simply act like put.
	// Gets values from the representation and
	// updates the internal state
	OCRepresentation post(OCRepresentation& rep)
	{
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

		return get_sensor();
	}
#endif


	// gets the updated representation.
	OCRepresentation get_sensor()
	{
		sensor_temp = sensor_read_temp();
		sensor_humidity = sensor_read_humidity();
		sensor_light = sensor_read_light();
		sensor_sound = sensor_read_sound();

		sensor_rep.setValue("temperature", sensor_temp);
		sensor_rep.setValue("humidity", sensor_humidity);
		sensor_rep.setValue("light", sensor_light);
		sensor_rep.setValue("sound", sensor_sound);
		return sensor_rep;
	}

	OCRepresentation get_led()
	{
		led_rep.setValue("red", led_red);
		led_rep.setValue("green", led_green);
		led_rep.setValue("blue", led_blue);
		return led_rep;
	}

	OCRepresentation get_lcd()
	{
		lcd_rep.setValue("lcd", lcd_str);
		return lcd_rep;
	}

	OCRepresentation get_buzzer()
	{
		buzzer_rep.setValue("buzzer", buzzer);
		return buzzer_rep;
	}

	OCRepresentation get_button()
	{
		button_rep.setValue("button", button);
		return button_rep;
	}

#if 0
	void addType(const std::string& type) const
	{
		OCStackResult result = OCPlatform::bindTypeToResource(sensor_resourceHandle, type);
		if (OC_STACK_OK != result)
			cout << "Binding TypeName to Resource was unsuccessful\n";
	}

	void addInterface(const std::string& interface) const
	{
		OCStackResult result = OCPlatform::bindInterfaceToResource(sensor_resourceHandle, interface);
		if (OC_STACK_OK != result)
			cout << "Binding TypeName to Resource was unsuccessful\n";
	}
#endif

private:
	// This is just a sample implementation of entity handler.
	// Entity handler can be implemented in several ways by the manufacturer
	OCEntityHandlerResult sensor_entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
		cout << "\tIn Server sensor entity handler:\n";
		OCEntityHandlerResult ehResult = OC_EH_ERROR;
		if(request) {
			// Get the request type and request flag
			std::string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

			if(requestFlag & RequestHandlerFlag::RequestFlag) {
				cout << "\t\trequestFlag : Request\n";
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());

				// Check for query params (if any)
				QueryParamsMap queries = request->getQueryParameters();

				if (!queries.empty())
					std::cout << "\nQuery processing upto entityHandler" << std::endl;

				for (auto it : queries) {
					std::cout << "Query key: " << it.first << " value : " << it.second << std:: endl;
				}

				if(requestType == "GET")
				{
					cout << "\t\t\trequestType : GET\n";
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_sensor());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
						ehResult = OC_EH_OK;
				} else if(requestType == "PUT") {
					cout << "\t\t\trequestType : PUT\n";
					cout << "Sensors don't have PUT method" << endl;
				} else if(requestType == "POST") {
					cout << "\t\t\trequestType : POST\n";
					cout << "Sensors don't have POST method" << endl;
				} else if(requestType == "DELETE") {
					cout << "Delete request received" << endl;
				}
			}

			if(requestFlag & RequestHandlerFlag::ObserverFlag) {
				ObservationInfo observationInfo = request->getObservationInfo();
				if(ObserveAction::ObserveRegister == observationInfo.action) {
					m_interestedObservers.push_back(observationInfo.obsId);
				} else if(ObserveAction::ObserveUnregister == observationInfo.action) {
					m_interestedObservers.erase(std::remove(
							m_interestedObservers.begin(),
							m_interestedObservers.end(),
							observationInfo.obsId),
							m_interestedObservers.end());
				}
			}
		} else {
			std::cout << "Request invalid" << std::endl;
		}

		return ehResult;
	}

	OCEntityHandlerResult led_entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
		cout << "\tIn Server CPP entity handler:\n";
		OCEntityHandlerResult ehResult = OC_EH_ERROR;

		if(request) {
			// Get the request type and request flag
			std::string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

			if(requestFlag & RequestHandlerFlag::RequestFlag) {
				cout << "\t\trequestFlag : Request\n";
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());

				// Check for query params (if any)
				QueryParamsMap queries = request->getQueryParameters();

				if (!queries.empty()) {
					std::cout << "\nQuery processing upto entityHandler" << std::endl;
				}

				for (auto it : queries) {
					std::cout << "Query key: " << it.first << " value : " << it.second << std:: endl;
				}

				if(requestType == "GET") {
					cout << "\t\t\trequestType : GET\n";
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_led());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "PUT") {
					cout << "\t\t\trequestType : PUT\n";
					OCRepresentation rep = request->getResourceRepresentation();

					// Do related operations related to PUT request
					// Update the lightResource
					put_led(rep);
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_led());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "POST") {
					cout << "\t\t\trequestType : POST\n";
#if 0
					OCRepresentation rep = request->getResourceRepresentation();

					// Do related operations related to POST request
					OCRepresentation rep_post = post(rep);
					pResponse->setResourceRepresentation(rep_post);
					pResponse->setErrorCode(200);
					if(rep_post.hasAttribute("createduri")) {
						pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
						pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
					} else {
						pResponse->setResponseResult(OC_EH_OK);
					}

					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
						ehResult = OC_EH_OK;
#endif
				} else if(requestType == "DELETE") {
					cout << "Delete request received" << endl;
				}
			}

			if(requestFlag & RequestHandlerFlag::ObserverFlag) {
				ObservationInfo observationInfo = request->getObservationInfo();

				if(ObserveAction::ObserveRegister == observationInfo.action) {
					m_interestedObservers.push_back(observationInfo.obsId);
				} else if(ObserveAction::ObserveUnregister == observationInfo.action) {
					m_interestedObservers.erase(std::remove(
							m_interestedObservers.begin(),
							m_interestedObservers.end(),
							observationInfo.obsId),
							m_interestedObservers.end());
				}
			}
		} else {
			std::cout << "Request invalid" << std::endl;
		}

		return ehResult;
	}

	OCEntityHandlerResult lcd_entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
		cout << "\tIn Server CPP entity handler:\n";
		OCEntityHandlerResult ehResult = OC_EH_ERROR;

		if(request) {
			// Get the request type and request flag
			std::string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

			if(requestFlag & RequestHandlerFlag::RequestFlag) {
				cout << "\t\trequestFlag : Request\n";
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());

				// Check for query params (if any)
				QueryParamsMap queries = request->getQueryParameters();

				if (!queries.empty()) {
					std::cout << "\nQuery processing upto entityHandler" << std::endl;
				}

				for (auto it : queries) {
					std::cout << "Query key: " << it.first << " value : " << it.second << std:: endl;
				}

				if(requestType == "GET") {
					cout << "\t\t\trequestType : GET\n";
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_lcd());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "PUT") {
					cout << "\t\t\trequestType : PUT\n";
					OCRepresentation rep = request->getResourceRepresentation();

					// Do related operations related to PUT request
					// Update the lightResource
					put_lcd(rep);
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_lcd());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "POST") {
					cout << "\t\t\trequestType : POST\n";
#if 0
					OCRepresentation rep = request->getResourceRepresentation();

					// Do related operations related to POST request
					OCRepresentation rep_post = post(rep);
					pResponse->setResourceRepresentation(rep_post);
					pResponse->setErrorCode(200);
					if(rep_post.hasAttribute("createduri")) {
						pResponse->setResponseResult(OC_EH_RESOURCE_CREATED);
						pResponse->setNewResourceUri(rep_post.getValue<std::string>("createduri"));
					} else {
						pResponse->setResponseResult(OC_EH_OK);
					}

					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse))
						ehResult = OC_EH_OK;
#endif
				} else if(requestType == "DELETE") {
					cout << "Delete request received" << endl;
				}
			}

			if(requestFlag & RequestHandlerFlag::ObserverFlag) {
				ObservationInfo observationInfo = request->getObservationInfo();

				if(ObserveAction::ObserveRegister == observationInfo.action) {
					m_interestedObservers.push_back(observationInfo.obsId);
				} else if(ObserveAction::ObserveUnregister == observationInfo.action) {
					m_interestedObservers.erase(std::remove(
							m_interestedObservers.begin(),
							m_interestedObservers.end(),
							observationInfo.obsId),
							m_interestedObservers.end());
				}
			}
		} else {
			std::cout << "Request invalid" << std::endl;
		}

		return ehResult;
	}

	OCEntityHandlerResult buzzer_entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
		cout << "\tIn Server CPP entity handler:\n";
		OCEntityHandlerResult ehResult = OC_EH_ERROR;

		if(request) {
			// Get the request type and request flag
			std::string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

			if(requestFlag & RequestHandlerFlag::RequestFlag) {
				cout << "\t\trequestFlag : Request\n";
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());

				// Check for query params (if any)
				QueryParamsMap queries = request->getQueryParameters();

				if (!queries.empty()) {
					std::cout << "\nQuery processing upto entityHandler" << std::endl;
				}

				for (auto it : queries) {
					std::cout << "Query key: " << it.first << " value : " << it.second << std:: endl;
				}

				if(requestType == "GET") {
					cout << "\t\t\trequestType : GET\n";
				} else if(requestType == "PUT") {
					cout << "\t\t\trequestType : PUT\n";
					OCRepresentation rep = request->getResourceRepresentation();

					// Do related operations related to PUT request
					// Update the lightResource
					put_buzzer(rep);
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_buzzer());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "POST") {
					cout << "\t\t\trequestType : POST\n";
				} else if(requestType == "DELETE") {
					cout << "Delete request received" << endl;
				}
			}

			if(requestFlag & RequestHandlerFlag::ObserverFlag) {
				ObservationInfo observationInfo = request->getObservationInfo();

				if(ObserveAction::ObserveRegister == observationInfo.action) {
					m_interestedObservers.push_back(observationInfo.obsId);
				} else if(ObserveAction::ObserveUnregister == observationInfo.action) {
					m_interestedObservers.erase(std::remove(
							m_interestedObservers.begin(),
							m_interestedObservers.end(),
							observationInfo.obsId),
							m_interestedObservers.end());
				}
			}
		} else {
			std::cout << "Request invalid" << std::endl;
		}

		return ehResult;
	}

	OCEntityHandlerResult button_entityHandler(std::shared_ptr<OCResourceRequest> request)
	{
		cout << "\tIn Server CPP entity handler:\n";
		OCEntityHandlerResult ehResult = OC_EH_ERROR;

		if(request) {
			// Get the request type and request flag
			std::string requestType = request->getRequestType();
			int requestFlag = request->getRequestHandlerFlag();

			if(requestFlag & RequestHandlerFlag::RequestFlag) {
				cout << "\t\trequestFlag : Request\n";
				auto pResponse = std::make_shared<OC::OCResourceResponse>();
				pResponse->setRequestHandle(request->getRequestHandle());
				pResponse->setResourceHandle(request->getResourceHandle());

				// Check for query params (if any)
				QueryParamsMap queries = request->getQueryParameters();

				if (!queries.empty()) {
					std::cout << "\nQuery processing upto entityHandler" << std::endl;
				}

				for (auto it : queries) {
					std::cout << "Query key: " << it.first << " value : " << it.second << std:: endl;
				}

				if(requestType == "GET") {
					cout << "\t\t\trequestType : GET\n";
					pResponse->setErrorCode(200);
					pResponse->setResponseResult(OC_EH_OK);
					pResponse->setResourceRepresentation(get_button());
					if(OC_STACK_OK == OCPlatform::sendResponse(pResponse)) {
						ehResult = OC_EH_OK;
					}
				} else if(requestType == "PUT") {
					cout << "\t\t\trequestType : PUT\n";
				} else if(requestType == "POST") {
					cout << "\t\t\trequestType : POST\n";
				} else if(requestType == "DELETE") {
					cout << "Delete request received" << endl;
				}
			}

			if(requestFlag & RequestHandlerFlag::ObserverFlag) {
				ObservationInfo observationInfo = request->getObservationInfo();

				if(ObserveAction::ObserveRegister == observationInfo.action) {
					m_interestedObservers.push_back(observationInfo.obsId);
				} else if(ObserveAction::ObserveUnregister == observationInfo.action) {
					m_interestedObservers.erase(std::remove(
							m_interestedObservers.begin(),
							m_interestedObservers.end(),
							observationInfo.obsId),
							m_interestedObservers.end());
				}

				pthread_t threadId;

				cout << "\t\trequestFlag : Observer\n";
				gObservation = 1;

				static int startedThread = 0;

				// Start a thread to check button status
				// If we have not created the thread already, we will create one here.

				if(!startedThread) {
					pthread_create (&threadId, NULL, CheckButtonRepresentation, (void *)this);
					startedThread = 1;
				}
				ehResult = OC_EH_OK;
			}
		} else {
			std::cout << "Request invalid" << std::endl;
		}

		return ehResult;
	}

};



// CheckButtonRepresentaion is an observation function,
// which notifies any changes to the resource to stack
// via notifyObservers if button is pressed or released
void * CheckButtonRepresentation (void *param)
{
	DemoResource *pdemo = (DemoResource*) param;
	int status;

	// This function continuously monitors for the changes
	while (1) {
		sleep (2);

		if (gObservation) {
			// If under observation if there are any changes to the button resource
			// we call notifyObservors
			
			status = pdemo->button_read();
			if(status == -1) {
				cout << "Button status read failed" << endl;
			}

			cout << "\nButton status: " << status << endl;
			cout << "Current button status: " << pdemo->button << endl;

			if(pdemo->button != status) {
				pdemo->button = status;
				cout << "Notifying observers with resource handle: " << pdemo->button_resourceHandle << endl;

				OCStackResult result = OC_STACK_OK;

				if(isListOfObservers) {
					std::shared_ptr<OCResourceResponse> resourceResponse =
						{std::make_shared<OCResourceResponse>()};

					resourceResponse->setErrorCode(200);
					resourceResponse->setResourceRepresentation(pdemo->get_button(), DEFAULT_INTERFACE);

					result = OCPlatform::notifyListOfObservers(pdemo->button_resourceHandle,
						pdemo->m_interestedObservers, resourceResponse);
				} else {
					result = OCPlatform::notifyAllObservers(pdemo->button_resourceHandle);
				}

				if(OC_STACK_NO_OBSERVERS == result) {
					cout << "No More observers, stopping notifications" << endl;
					gObservation = 0;
				}
			}
		}
	}

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
	pResponse->setResourceRepresentation(demoPtr->get_sensor());
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
	struct ifaddrs *ifaddr, *ifa;
	int s;
	char host_ip[NI_MAXHOST];
	std::string host_str;

	OCPersistentStorage ps {client_open, fread, fwrite, fclose, unlink };

	if (argc == 1) {
		isListOfObservers = false;
		isSecure = false;
	} else if (argc == 2) {
		int value = atoi(argv[1]);
		switch (value) {
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
	} else {
		return -1;
	}

	if (getifaddrs(&ifaddr) == -1) {
		std::cout << "getifaddrs error" << std::endl;
		return -1;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		if(!strcmp(ifa->ifa_name, "eth0") || !strcmp(ifa->ifa_name, "wlan0")) {
			if(ifa->ifa_addr->sa_family == AF_INET) {
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host_ip,
					NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s) {
					std::cout << " getnameinfo fail: " << gai_strerror(s) << std::endl;
					freeifaddrs(ifaddr);
					return -1;
				}
				//host_str = ifa->ifa_name;
				host_str = "IPv4: ";
				host_str += host_ip;
				std::cout << host_str << std::endl;
				break;
			} else if(ifa->ifa_addr->sa_family == AF_INET6) {
				s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host_ip,
					NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if(s) {
					std::cout << " getnameinfo fail: " << gai_strerror(s) << std::endl;
					freeifaddrs(ifaddr);
					return -1;
				}
				//host_str = ifa->ifa_name;
				host_str = "IPv6: ";
				host_str += host_ip;
				std::cout << host_str << std::endl;
			}
		}
	}
	freeifaddrs(ifaddr);

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
	try {
		// Create the instance of the resource class
		DemoResource myDemo;

		// Invoke createResource function of class light.
		myDemo.createResource();
		std::cout << "Created resource." << std::endl;
	
		myDemo.lcd_write_str(host_str.c_str());
		myDemo.lcd_str = host_str;	
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
	catch(OCException &e) {
		std::cout << "OCException in main : " << e.what() << endl;
	}

	// No explicit call to stop the platform.
	// When OCPlatform::destructor is invoked, internally we do platform cleanup

	return 0;
}
