// Copyright 2016 Samuel Austin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Camera SDK includes
#include "cameralibrary.h"
#include "modulevector.h"
#include "modulevectorprocessing.h"
#include "coremath.h"

class trackir
{
	public:
		trackir(OSVR_PluginRegContext ctx, CameraLibrary::Camera *camera);
		~trackir();
		
		OSVR_ReturnCode update();

	private:
		osvr::pluginkit::DeviceToken m_dev;
		OSVR_TrackerDeviceInterface m_tracker;

		CameraLibrary::cModuleVector *m_vec;
		CameraLibrary::cModuleVectorProcessing *m_vecprocessor;
		CameraLibrary::Camera *m_camera;
		Core::DistortionModel m_lensDistortion;

		bool m_vectorDetected;
		int m_frameCount;

		// Position and orientation returned to game
		double m_x, m_y, m_z;
		double m_yaw, m_pitch, m_roll;
};