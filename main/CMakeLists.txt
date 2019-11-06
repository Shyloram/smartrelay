set(COMPONENT_SRCS "main.c"
                   "softap_prov.c"
                   "tcp_server.c"
                   "wifilib.c"
                   "aws_proc.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
