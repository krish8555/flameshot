cmake_minimum_required(VERSION 3.22)

set(ML_FILES
    objectdetector.cpp
    objectdetector.h
    onnxdetector.cpp
    onnxdetector.h
)

if(ENABLE_ONNX_ML)
    add_library(flameshot_ml STATIC ${ML_FILES})
    
    target_link_libraries(flameshot_ml
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
    )
    
    # Find ONNX Runtime
    find_path(ONNXRUNTIME_INCLUDE_DIR
        NAMES onnxruntime_cxx_api.h
        PATHS
            /usr/include/onnxruntime
            /usr/local/include/onnxruntime
            ${ONNXRUNTIME_ROOT}/include
    )
    
    find_library(ONNXRUNTIME_LIBRARY
        NAMES onnxruntime libonnxruntime
        PATHS
            /usr/lib
            /usr/local/lib
            ${ONNXRUNTIME_ROOT}/lib
    )
    
    if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIBRARY)
        target_include_directories(flameshot_ml PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
        target_link_libraries(flameshot_ml ${ONNXRUNTIME_LIBRARY})
        target_compile_definitions(flameshot_ml PUBLIC ENABLE_ONNX_ML)
        message(STATUS "ONNX Runtime found: ${ONNXRUNTIME_LIBRARY}")
    else()
        message(WARNING "ONNX Runtime not found. Smart selection will be unavailable.")
        message(STATUS "Set ONNXRUNTIME_ROOT to the ONNX Runtime installation directory")
    endif()
    
    target_include_directories(flameshot_ml PUBLIC ${CMAKE_SOURCE_DIR}/src)
endif()
