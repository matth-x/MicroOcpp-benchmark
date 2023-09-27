idf_component_register(
    SRCS "main.c" "cxx_intf.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_timer nvs_flash mongoose MicroOcpp MicroOcppMongoose
)
