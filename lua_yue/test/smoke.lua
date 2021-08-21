local dir = arg[1]
package.cpath = dir .. '/?.dll;' .. dir ..'/?.so'

local gui = require('yue.gui')

local win = gui.Window.create({})
win:close()

gui.MessageLoop.posttask(function()
  gui.MessageLoop.quit()
end)

gui.MessageLoop.run()
