SET CMAKE_PREFIX_PATH d:\proj64_cmake
SET BOOST_ROOT %CMAKE_PREFIX_PATH%\boost\boost_1_72_0
SET BOOST_LIBRARYDIR %BOOST_ROOT%lib64-msvc-14.2
SET BUILD_MINIMUM_MODULES OFF
python3 -m pip install -v -e .
