add_executable(${ProjectName}
    main.c
    drivers/ledonboard/leds.c
    drivers/uart/uart.c
    pi7/comm_pc/modbus.c
    pi7/comm_pic/comm_pic.c
    pi7/command_interpreter/command_interpreter.c
    pi7/trj_control/trj_control.c
    pi7/trj_program/trj_program.c
    pi7/trj_state/trj_state.c
)

target_include_directories(${ProjectName} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/drivers/ledonboard
    ${CMAKE_CURRENT_LIST_DIR}/drivers/uart
    ${CMAKE_CURRENT_LIST_DIR}/pi7/comm_pc
    ${CMAKE_CURRENT_LIST_DIR}/pi7/comm_pic
    ${CMAKE_CURRENT_LIST_DIR}/pi7/command_interpreter
    ${CMAKE_CURRENT_LIST_DIR}/pi7/trj_control
    ${CMAKE_CURRENT_LIST_DIR}/pi7/trj_program
    ${CMAKE_CURRENT_LIST_DIR}/pi7/trj_state
)

target_link_libraries(${ProjectName} 
    pico_stdlib 
    pico_multicore
    pico_cyw43_arch_none
    FreeRTOS-Kernel-Heap4 
    )
    
pico_enable_stdio_usb(${ProjectName} 1)
pico_enable_stdio_uart(${ProjectName} 0)
    
pico_add_extra_outputs(${ProjectName})