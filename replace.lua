-- https://github.com/wlupton/pandoc-lua-logging/
local logging = require "logging"

-- local function helper_CodeBlockImage(elem)
-- 	if elem.caption[1].text == "include" then
-- 		local filename = elem.src
		
-- 		-- get file extension
-- 		local extension = ""
-- 		local extensionIndex = string.find(filename, "%.%w+$")
-- 		if not extensionIndex then
-- 			logging.warning("Couldn't extract extension!", filename)
-- 		else
-- 			extension = string.sub(filename, extensionIndex + 1)
-- 		end
		
-- 		local filehandle = io.open(filename, "r")
		
-- 		-- get file contents
-- 		local contents = ""
-- 		if not filehandle then
-- 			logging.warning("Couldn't load file!", filename)
-- 		else
-- 			contents = filehandle:read('*a')
-- 			filehandle:close()
-- 		end
		
-- 		-- make a code block!!
-- 		return pandoc.CodeBlock(
-- 			contents, extension
-- 		)
-- 	end
-- 	return elem
-- end

-- function Para(elem)
-- 	if #elem.content == 1 then
-- 		local inner = elem.content[1]
-- 		if inner.tag == "Image" then
-- 			return helper_CodeBlockImage(inner)
-- 		end
-- 	end
-- 	return elem
-- end

-- https://groups.google.com/g/pandoc-discuss/c/mj3G_DRloCs/m/ncZVJaeGAwAJ
function CodeBlock(elem)
	local include = elem.attributes.include
	if include then
		local filename = include
		
		-- get file extension
		local extension = ""
		local extensionIndex = string.find(filename, "%.%w+$")
		if not extensionIndex then
			logging.warning("Couldn't extract extension!", filename)
		else
			extension = string.sub(filename, extensionIndex + 1)
		end
		
		local filehandle = io.open(filename, "r")
		
		-- get file contents
		local contents = ""
		if not filehandle then
			logging.warning("Couldn't load file!", filename)
		else
			contents = filehandle:read('*a')
			filehandle:close()
		end
		
		-- make a code block!!
		return pandoc.CodeBlock(
			contents,
			pandoc.Attr(
				elem.identifier,
				elem.classes,
				elem.attributes
			)
		)
	end
end

-- echo '![include](./01a-segfault.c)' | pandoc -t native
-- echo '![include](./01a-segfault.c)' | pandoc --lua-filter=./replace.lua -t native
-- https://pandoc.org/lua-filters.html#macro-substitution
