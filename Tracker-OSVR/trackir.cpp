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

#include <Windows.h>

// Internal Includes
#include <osvr/PluginKit/PluginKit.h>
#include <osvr/PluginKit/TrackerInterfaceC.h>

// Generated JSON header file
#include "com_samaust_trackerv2_osvr_json.h"

// Standard includes
#include <iostream>
#include <cmath>

#include "trackir.h"

#define DEGTORADDIV2 0.00872664625997165

trackir::trackir(OSVR_PluginRegContext ctx, CameraLibrary::Camera *camera)
{
	std::cout << "[TrackerV2-OSVR] Initializing TrackIR..." << std::endl;

	m_camera = camera;

	// Set Video Mode
	// TrackIR 4 only supports segment mode
	// TrackIR 5 supports segment mode, Interleaved grayscale mode - non-realtime and Bit Packed Precision mode
	m_camera->SetVideoType(Core::SegmentMode);

	// Start camera output
	m_camera->Start();

	// CameraLibrary objects
	m_vec = CameraLibrary::cModuleVector::Create(); //new cModuleVector();
	m_vecprocessor = new CameraLibrary::cModuleVectorProcessing();
	CameraLibrary::cVectorProcessingSettings vectorProcessorSettings;
	CameraLibrary::cVectorSettings vectorSettings;

	// Get Vector arrangement from settings (currently hardcoded to TrackClipPro)
	//CameraLibrary::cVectorSettings::Arrangements selectedArrangement = CameraLibrary::cVectorSettings::VectorClip;
	CameraLibrary::cVectorSettings::Arrangements selectedArrangement = CameraLibrary::cVectorSettings::TrackClipPro;
	
	// Configure vectorProcessorSettings
	vectorProcessorSettings = *m_vecprocessor->Settings();

	vectorProcessorSettings.Arrangement = selectedArrangement;
	vectorProcessorSettings.ShowPivotPoint = false;
	vectorProcessorSettings.ShowProcessed = false;

	m_vecprocessor->SetSettings(vectorProcessorSettings);

	// Get camera lens distortion
	m_camera->GetDistortionModel(m_lensDistortion);

	// Configure vector settings
	vectorSettings = *m_vec->Settings();
	vectorSettings.Arrangement = selectedArrangement;
	vectorSettings.Enabled = true;

	// Plug in focal length in (mm) by converting it from pixels -> mm
	vectorSettings.ImagerFocalLength = (m_lensDistortion.HorizontalFocalLength / ((float)m_camera->PhysicalPixelWidth()))*m_camera->ImagerWidth();

	vectorSettings.ImagerHeight = m_camera->ImagerHeight();
	vectorSettings.ImagerWidth = m_camera->ImagerWidth();

	vectorSettings.PrincipalX = m_camera->PhysicalPixelWidth() / 2;
	vectorSettings.PrincipalY = m_camera->PhysicalPixelHeight() / 2;

	vectorSettings.PixelWidth = m_camera->PhysicalPixelWidth();
	vectorSettings.PixelHeight = m_camera->PhysicalPixelHeight();

	m_vec->SetSettings(vectorSettings);

	// Turn off TrackIR illumination LEDs (not necessary for TrackClip PRO)
	m_camera->SetLED(CameraLibrary::eStatusLEDs::IlluminationLED, false);

	std::cout << "[TrackerV2-OSVR] TrackIR initialized" << std::endl;

	// Create the initialization options
	OSVR_DeviceInitOptions opts = osvrDeviceCreateInitOptions(ctx);

	// configure our tracker
	osvrDeviceTrackerConfigure(opts, &m_tracker);

	// Create the device token with the options
	m_dev.initAsync(ctx, "Tracker", opts);

	// Send JSON descriptor
	m_dev.sendJsonDescriptor(com_samaust_trackerv2_osvr_json);

	// Register update callback
	m_dev.registerUpdateCallback(this);
}

trackir::~trackir(void)
{
	// Release camera
	m_camera->Release();

	// Shutdown Camera Library
	CameraLibrary::CameraManager::X().Shutdown();
}

OSVR_ReturnCode trackir::update() {
	// Fetch a new frame from the camera
	CameraLibrary::Frame *frame = m_camera->GetFrame();

	if (frame)
	{
		m_vec->BeginFrame();
		int frameObjectCount = frame->ObjectCount();

		// Light green LED if TracClip PRO is detected
		if (frameObjectCount >= 3)
			m_camera->SetLED(CameraLibrary::eStatusLEDs::GreenStatusLED, true);
		else
			m_camera->SetLED(CameraLibrary::eStatusLEDs::GreenStatusLED, false);

		for (int i = 0; i<frameObjectCount; i++)
		{
			CameraLibrary::cObject *obj = frame->Object(i);

			float x = obj->X();
			float y = obj->Y();

			Core::Undistort2DPoint(m_lensDistortion, x, y);

			m_vec->PushMarkerData(x, y, obj->Area(), obj->Width(), obj->Height());
		}
		m_vec->Calculate();
		m_vecprocessor->PushData(m_vec);

		// Recenter if CTRL + F12 is pressed
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			if (GetAsyncKeyState(VK_F12) & 0x8000)
				m_vecprocessor->Recenter();
		}

		// Get position and orientation
		m_vecprocessor->GetPosition(m_x, m_y, m_z);
		m_vecprocessor->GetOrientation(m_yaw, m_pitch, m_roll);

		// Release frame
		frame->Release();
	}

	// Euler to quaternion
	// Source : http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
	double c1 = cos(m_yaw * DEGTORADDIV2);
	double s1 = sin(m_yaw * DEGTORADDIV2);
	double c2 = cos(m_pitch * DEGTORADDIV2);
	double s2 = sin(m_pitch * DEGTORADDIV2);
	double c3 = cos(-m_roll * DEGTORADDIV2);
	double s3 = sin(-m_roll * DEGTORADDIV2);
	double c1c2 = c1*c2;
	double s1s2 = s1*s2;

	// Report pose
	OSVR_Pose3 pose;
	pose.translation.data[0] = m_x;
	pose.translation.data[1] = m_y;
	pose.translation.data[2] = m_z;
	pose.rotation.data[0] = c1c2*c3 - s1s2*s3;
	pose.rotation.data[1] = c1c2*s3 + s1s2*c3;
	pose.rotation.data[2] = s1*c2*c3 + c1*s2*s3;
	pose.rotation.data[3] = c1*s2*c3 - s1*c2*s3;
	osvrDeviceTrackerSendPose(m_dev, m_tracker, &pose, 0);

	return OSVR_RETURN_SUCCESS;
}
