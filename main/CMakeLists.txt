# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "app.cpp" "webserver.cpp" "wifiap.c" "main.c")
set(COMPONENT_SRCS "webserver.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
