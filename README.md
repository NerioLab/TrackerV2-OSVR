# TrackerV2-OSVR
TrackIR tracking plugin for OSVR based on OptiTrack Camera SDK v1.9.0

## Important notice

The current performance is not satisfactory to be recommended for use. There are issues with rotation and position interacting with each other, rotation suddenly snapping to a wrong angle and jitter that would require to add some filtering. The Camera SDK lacks documentation and does not include the source code so it is hard to figure out the cause of these issues and they may never be resolved. It might be a better idea to make a plugin with the TrackIR SDK since the orientations and positions reported in TrackIR software work properly as intended.

## Instructions for 32 bit version

Copy TrackerV2-OSVR\build_x86\bin\osvr-plugins-0\Release\com_samaust_trackerv2_osvr.dll to osvr-plugins-0 folder.

Copy CameraLibrary2010D.dll to osvr_server.exe folder.

## Instructions for 64 bit version

Copy TrackerV2-OSVR\build_x64\bin\osvr-plugins-0\Release\com_samaust_trackerv2_osvr.dll to osvr-plugins-0 folder.

Copy CameraLibrary2010x64D.dll to osvr_server.exe folder.

## Shorcuts

Recenter : CTRL + F12

## Notes

There are currently no settings to scale the positions and orientations.
TrackClip PRO support is hardcoded. A future version will let the user choose between the Vector Clip and TrackClip PRO

## How to compile

The provided solutions are for Visual Studio 2013. You can generate new solutions for other compilers using CMake : 

* Set the source folder to Tracker-OSVR
* Set the build folder
* Add entry CMAKE_PREFIX_PATH and set it to the OSVR-Core Binary Snapshot folder
* Configure and generate

To compile:

* Download and extract an OSVR-Core Binary Snapshot.
* Modify the additional include directory project property to point to the OSVR-Core Binary Snapshot folder.
* Download the OptiTrack SDK and install it. It will automatically set the NP_CAMERASDK environment variable to point to the SDK install folder. The project settings use that environment variable to find the Camera SDK include and library files.
* Get a copy of boost and modify the additional include directory project property. I installed boost using OSVR-Boxstarter but if you don't need to setup a whole development environment, you can directly download it from its official website.
* Modify the additional include directory project property of the build folder.
* Compile
