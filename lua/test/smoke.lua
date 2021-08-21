local dir = arg[1]
package.cpath = dir .. '/?.dll;' .. dir ..'/?.so'

os.exit(require('lua_unittests'))
