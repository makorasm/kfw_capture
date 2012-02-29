# Defines properties of the example/demo/application

# Max resolution supported in this app
MAX_RESOLUTION = hd

# Application Type
#     vs: video surveillance
APP_TYPE = vc

# IPC Modes: 
#     local: only this core 
#     remote: only intra-ducati (between two M3 cores)
#     remoteWithHOST: ducati cores and host (A8)
IPC_MODE = remoteWithHOST

# Codecs required for this demo:
#     Eg: h264enc mpeg2enc...
CODECS_IN_APP = h264enc 

# Nothing beyond this point
