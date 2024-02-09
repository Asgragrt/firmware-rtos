add_executable(${ProjectName}
    main.c
    tusb/tud_utils.c
    kbd/kbd.c
    leds/leds.c
    leds/leds_utils.c
    flash/flash.c
)


target_sources(${PROJECT_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/tusb/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/freertos_hook.c
    )

target_include_directories(${ProjectName} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# target_compile_definitions(${ProjectName} PRIVATE PICO_MALLOC_PANIC=0)

target_link_libraries(${ProjectName} 
    pico_stdlib
    pico_flash
    hardware_pwm
    hardware_flash 
    tinyusb_device
    FreeRTOS-Kernel-Heap4 
    )

pico_add_extra_outputs(${ProjectName})

target_compile_definitions(${ProjectName} PRIVATE
PICO_FLASH_SAFE_EXECUTE_PICO_SUPPORT_MULTICORE_LOCKOUT=0
PICO_FLASH_SAFE_EXECUTE_USE_FREERTOS_SMP=1
)