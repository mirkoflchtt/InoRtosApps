if(IDF_TARGET STREQUAL "esp8266")
    # Tell CMake to link the ESP8266 SPI Flash peripheral system
    idf_component_register(SRCS "main.cpp"
                           INCLUDE_DIRS "." "include"
                           REQUIRES InoRtosLib spi_flash pthread)
else()
    idf_component_register(SRCS "main.cpp"
                           INCLUDE_DIRS "." "include"
                           REQUIRES InoRtosLib esp_driver_gpio spi_flash)
endif()
