# the exercise name is nothing else than the directory
get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

include_directories(constants)
include_directories(imgui)

# these files are copied only once
file(GLOB TEXTURES
    cube/cube_texture.tga
    cube/tex_sky_0.tga
    grid/grid_texture.tga
    water/*.png
    grid/*.png
    screenquad/*.png)
copy_files_once(${TEXTURES})

# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
  cube/cube_vshader.glsl
  cube/cube_fshader.glsl
  grid/grid_vshader.glsl
  grid/grid_fshader.glsl
  screenquad/normal_vshader.glsl
  screenquad/normal_fshader.glsl
  screenquad/screenquad_vshader.glsl
  screenquad/screenquad_fshader.glsl
  water/water_vshader.glsl
  water/water_fshader.glsl
  bird/bird_vshader.glsl
  bird/bird_fshader.glsl
  guiTextures/guiTextures_vshader.glsl
  guiTextures/guiTextures_fshader.glsl)
deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})
