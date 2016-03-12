// Copyright 2014 Sensics, Inc.
// Copyright 2016 Samuel Austin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Windows.h>

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

#include "cameralibrary.h"     //== Camera Library header file ======================---
#include "modulevector.h"
#include "modulevectorprocessing.h"
#include "coremath.h"

#include "trackir.h"

// Library/third-party includes
// - none

// Standard includes
#include <iostream>

// Anonymous namespace to avoid symbol collision
namespace {

class HardwareDetection {
  public:
    HardwareDetection() : m_found(false) {}
    OSVR_ReturnCode operator()(OSVR_PluginRegContext ctx) {
		std::cout << "[TrackerV2-OSVR] Detecting TrackIR..." << std::endl;
        if (!m_found) {
			// Initialize Camera SDK
			CameraLibrary::CameraManager::X();

			// At this point the Camera SDK is actively looking for all connected cameras and will initialize
			// them on it's own.

			Sleep(3000); // Some delay is necessary to initialize the camera
			
			// Get a connected camera
			CameraLibrary::Camera *camera = CameraLibrary::CameraManager::X().GetCamera();

			if (camera == 0)
			{
				std::cout << "[TrackerV2-OSVR] TrackIR not detected" << std::endl;
				return OSVR_RETURN_FAILURE;
			}

            m_found = true;

			std::cout << "[TrackerV2-OSVR] TrackIR detected" << std::endl;
            /// Create our device object
            osvr::pluginkit::registerObjectForDeletion(
				ctx, new trackir(ctx, camera));
        }
        return OSVR_RETURN_SUCCESS;
    }

  private:
    /// @brief Have we found our device yet? (this limits the plugin to one
    /// instance)
    bool m_found;
};
} // namespace

OSVR_PLUGIN(com_samaust_trackerv2_osvr) {
    osvr::pluginkit::PluginContext context(ctx);

	/// Register a detection callback function object.
    context.registerHardwareDetectCallback(new HardwareDetection());

    return OSVR_RETURN_SUCCESS;
}
