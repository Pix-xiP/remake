-- do some function stuff here perhaps?
-- identify the compiler?
-- locate dependencies?
local function foo()
	return "cc"
end

return {
	-- compiler = "cc",
	compiler = foo,
	name = "remake_test",
	-- This will be prefix'd with -D
	defines = {
		"DEBUG",
	},
	-- CLFAGS will be apppended as is for now
	cflags = {
		"-ggdb",
		"-fPIC",
		"-fsanitize=address",
	},
	-- As it says on the tin.
	src_files = {
		"./src/test.c",
		"./src/extended.c",
	},
	-- Directories that could contain header files to include.
	include_dirs = {
		"./hdr",
		"./temp",
	},
	-- Libraries to link against!
	include_libs = {
		"pthread",
		"mimalloc",
	},
	-- None, Basic, Default, Extreme, debug, debug_optimisied, size
	optimisation_level = "default",

	--------------------
	-- UNIMPLEMENTED YET
	-- Directory to put build files in? Maybe .o files?
	build_dir = "./build",
	-- Directory to install the binary to after the fact?
	install = {
		directory = "./bin",
	},
}
