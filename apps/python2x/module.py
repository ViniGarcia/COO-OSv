from osv.modules import api

api.require_running('httpserver')
api.require('cli')

#api.require('unknown-term')
#default = api.run(cmdline="--env=TERM=unknown /python")

