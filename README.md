# simple file <-> HTTP Web server for esp32

supports:
* basic file operations list/delete/upload
* synchronization of folders based on file hashes
* ota over http (optinonaly comparing time stamps to avoid unneeded uploads)



# using it

```#include "WebFS/WebFS.h"

const char *hostName = "myGreatHostName";
void setup() {
    ... init any wifi here first
    bool announceCaps=true;
    WebFS::setup(ws::server, hostName, announceCaps);
}

void loop() {
    WebFS::loop();
}
```


then going to http://myGreatHostName.local/m

will show basic interface for file manipulations

all uploaded files are also available at http://myGreatHostName.local/filePath.ext



# cli tools
set of tools to keep a fleet of esp's updated

per esp operations
```
source cliTools.sh 
espIp="myGreatHostName.local"
update_new_files path/to/dir
```

or automatically upgrade all connected to this network (announceCaps must be true)
```
source fleetTools.sh; 
upgradeAllESP /pathToBin/myBin.ino.bin /pathTo/public/dir
```


# known issues

* poor folder support for now
