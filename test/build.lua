-- do some function stuff here perhaps?
-- identify the compiler?
-- locate dependencies?
local function foo()
	return "cc"
end

return {
	-- compiler = "cc",
	compiler = foo,
	executable = "remake_test",
	-- This will be prefix'd with -D
	build_defines = {
		"DEBUG",
	},
	-- CLFAGS will be apppended as is for now
	cflags = {
		"-ggdb",
		"-fPIC",
	},
	-- As it says on the tin.
	src_files = {
		"./src/test.c",
	},
	-- Directories that could contain header files to include.
	include_dir = {
		"./hdr",
		"./temp",
	},
	-- Libraries to link against!
	include_libs = {
		"pthread",
		"mimalloc",
	},
	-- Directory to install the binary to after the fact?
	install = {
		directory = "./bin",
	},
	-- Directory to put build files in? Maybe .o files?
	build_dir = "./build",
}
