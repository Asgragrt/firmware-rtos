add_executable(${ProjectName}
    main.c
    tusb/tud_utils.c
    kbd/kbd.c
    switch/switch.c
    leds/leds.c
    leds/leds_utils.c
    flash/flash.c
)

# Specifying FreeRTOSConfig directory
add_compile_options(FREERTOS_CONFIG_FILE_DIRECTORY = ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS)

target_sources(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/tusb/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS/freertos_hook.c
    )

target_include_directories(${ProjectName} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)


# Linking needed libraries
target_link_libraries(${ProjectName} 
    pico_stdlib
    pico_flash
    pico_unique_id
    hardware_pwm
    hardware_flash 
    tinyusb_device
    FreeRTOS-Kernel-Heap4 
    )

pico_add_extra_outputs(${ProjectName})

target_compile_definitions(${ProjectName} PRIVATE
PICO_FLASH_SAFE_EXECUTE_PICO_SUPPORT_MULTICORE_LOCKOUT=0 # Needed for proper safe execute functioning
PICO_FLASH_SAFE_EXECUTE_USE_FREERTOS_SMP=1 # Needed for proper safe execute functioning
PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64 # Needed to compensate for bad PCB design
)