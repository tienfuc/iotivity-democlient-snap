//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

/**
 * @file
 *
 * This file contains the resource container APIs provided to the developers.
 */

#ifndef RESOURCECONTAINER_H_
#define RESOURCECONTAINER_H_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <list>

#include "BundleInfo.h"

namespace OIC
{
    namespace Service
    {

        /**
         * @class   ResourceContainer
         * @brief    This class provides APIs for managing the container and bundles in the container.
         *
         */
        class ResourceContainer
        {
            public:
                /**
                * Constructor
                */
                ResourceContainer();

                /**
                *virtual Destructor
                */
                virtual ~ResourceContainer();

                /**
                 * API for starting the Container
                 *
                 * @details This API start the container with the provided Configuration file.
                 *
                 * @param configFile - configuration File that contains the Bundle/Bundles information.
                 *
                 */
                virtual void startContainer(std::string configFile) = 0;
                /**
                * API for stopping the Container
                */
                virtual void stopContainer() = 0;

                // list of bundle ids
                /**
                * API for getting the list of all bundles in the container
                *
                * @return  list<BundleInfo*> -List of BundleInfo pointer each associated with a bundle
                *
                */
                virtual std::list<BundleInfo *> listBundles() = 0;
                /**
                 * API for starting the bundle.
                 *
                 * @param bundleId - Id of the Bundle
                 *
                 */
                virtual void startBundle(std::string bundleId) = 0;
                /**
                * API for Stopping the bundle
                *
                * @param bundleId - Id of the Bundle
                *
                */
                virtual void stopBundle(std::string bundleId) = 0;

                // dynamic configuration
                /**
                 * API for adding the bundle to the Container
                 *
                 * @param bundleId - Id of the Bundle
                 * @param bundleUri - Uri of the bundle
                 * @param bundlePath - Path of the bundle
                 * @param params  - key-value pairs in string form for other Bundle parameters
                 *
                 */
                virtual void addBundle(std::string bundleId, std::string bundleUri, std::string bundlePath,
                                       std::map<std::string, std::string> params) = 0;
                /**
                 * API for removing the bundle from the container
                 *
                 * @param bundleId - Id of the Bundle
                 *
                 */
                virtual void removeBundle(std::string bundleId) = 0;

                /**
                * API for adding the Resource configuration information to the bundle
                *
                * @param bundleId - Id of the Bundle
                * @param resourceUri - URI of the resource
                * @param params  - key-value pairs in string form for other Bundle parameters
                *
                */
                virtual void addResourceConfig(std::string bundleId, std::string resourceUri,
                                               std::map<std::string, std::string> params) = 0;
                /**
                * API for removing the Resource configuration information from the bundle
                *
                * @param bundleId - Id of the Bundle
                * @param resourceUri - URI of the resource
                *
                */
                virtual void removeResourceConfig(std::string bundleId, std::string resourceUri) = 0;

                /**
                * API for getting the list of Bundle Resources
                *
                * @param bundleId - Id of the Bundle
                *
                */
                virtual std::list<std::string> listBundleResources(std::string bundleId) = 0;

                /**
                 * API for getting the Instance of ResourceContainer class
                 *
                 * @return ResourceContainer - Instance of the "ResourceContainer" class
                 *
                 */
                static ResourceContainer *getInstance();
        };
    }
}

#endif /* RESOURCECONTAINER_H_ */