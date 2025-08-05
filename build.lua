-- Does this actually work? xD
local function check_compiler(compiler)
	local _, _, signal = os.execute(compiler .. "&>/dev/null")
	return signal == 1
end

local c_compilers = { "cc", "gcc", "clang", "tcc" }
-- do more function stuff here perhaps?
-- identify the compiler even better?
-- locate dependencies too?
local function fetch_compiler()
	local CC = os.getenv("CC")
	if CC ~= nil then
		if check_compiler(CC) then
			return CC
		end
		io.stderr:write("[WARNING]: CC is not set to an existing compiler (check your $PATH). Using a fallback.\n")
	end

	for _, c in ipairs(c_compilers) do
		check_compiler(c)
	end
end

return {
	compiler = "cc",
	-- compiler = fetch_compiler(),
	name = "pb",
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
		"src/pb_file_checker.c",
		"src/pb_main.c",
		"src/pb_parsing.c",
	},
	-- Directories that could contain header files to include.
	include_dirs = {
		"./src", -- Probs dont need up here
		"./lua-5.4.6/install/include",
		"./mimalloc/include",
	},
	-- Libraries to link against! (-l<lib>)
	include_libs = {
		"lua",
		"m",
	},
	library_dirs = {
		"./lua-5.4.6/install/lib",
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
