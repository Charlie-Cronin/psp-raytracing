cmake_minimum_required(VERSION 3.11)

project(Raytracing)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pspdebug
    pspdisplay
    pspge
    pspgu
    pspctrl
    psppower
)

# create an EBOOT.PBP file
create_pbp_file(
    TARGET ${PROJECT_NAME} 
    # icon0 and pic0, important to note later
    ICON_PATH NULL
    BACKGROUND_PATH NULL
    PREVIEW_PATH NULL
    TITLE ${PROJECT_NAME}
    VERSION 01.00
)