
list(APPEND test_names
    base_test.cpp
    view_test.cpp
    mesh_test.cpp
    texture_test.cpp
    texturedmesh_test.cpp
    pointcloud_test.cpp
    imagedisplay_test.cpp
    userinput_test.cpp
    glfwinput_test.cpp
    dual_window.cpp
    gl_mesh_test.cpp
    glsl_types.cpp
    fan_renderer.cpp
    instances_renderer.cpp
    png_codec.cpp
    obj_loader.cpp
)
list(APPEND test_deps
    rtac_display
)

foreach(name ${test_names})

    # Generating a unique target name to avoid name colision with other rtac
    # packages.
    get_filename_component(executable_name ${name} NAME_WE)
    set(test_target_name ${executable_name}_${PROJECT_NAME})

    add_executable(${test_target_name} src/${name})
    target_link_libraries(${test_target_name} ${test_deps})
    set_target_properties(${test_target_name} PROPERTIES OUTPUT_NAME ${executable_name})

endforeach()

if(TARGET Freetype::Freetype)
    add_subdirectory(freetype_test)
endif()

add_subdirectory(reductions)


