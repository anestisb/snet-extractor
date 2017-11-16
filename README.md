# Snet-Extractor

Command line tool to download and extract the latest version of the Google Play Services SafetyNet
Jar file.

To use simply execute the `run.sh` script and the latest version of the bytecode will be downloaded
in the current directory, along with all the metadata.

```bash
$ ./run.sh
[*] Downloading SNET flags file
######################################################################## 100.0%
[*] Successfully extracted 'metadata_flags.txt'
[*] Successfully extracted 'payload.snet'
[*] Detected snet version '10001000'
[*] Downloading SNET Jar file
######################################################################## 100.0%
[*] Successfully extracted 'metadata_flags.txt'
[*] Successfully extracted 'payload.snet'
[*] All files successfully extract at 'snet-10001000'

$ tree snet-10001000/
snet-10001000/
├── 03242017-10001000.snet
├── metadata_flags.txt
├── snet.flags
├── snet.jar
└── snet_flags.json

0 directories, 5 files
```


## License

```
   Anestis Bechtsoudis <anestis@census-labs.com>
   Copyright 2017 by CENSUS S.A. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```
