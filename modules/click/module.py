from osv.modules.api import *
from osv.modules.filemap import FileMap
from osv.modules import api

require_running('httpserver')
require('cli')

usr_files = FileMap()
usr_files.add('${OSV_BASE}/modules/click').to('/') \
	.include('libintel_dpdk.so') \
	.include('click') \
	.include('func.click') \
        .include('func.exe')

full = api.run('--verbose --maxnic=1 /cli/cli.so')
#full = api.run('/click --dpdk --no-shconf -c 0x01 -n 1 --log-level 8 -m 64 -- -p 8001 func.click')
default = full
