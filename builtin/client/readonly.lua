-- Set the global environment to a read-only mode.

local f=function (t,i) error("cannot redefine global variable `"..i.."'",2) end
local g={}
local G=getfenv()
setmetatable(g,{__index=G,__newindex=f})
setfenv(1,g)
