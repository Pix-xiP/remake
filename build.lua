-- do more function stuff here perhaps?
-- identify the compiler even better?
-- locate dependencies too?

-- This is like, wayyy more reliable, totes!
local function compiler_exists(compiler)
	return os.execute("command -v " .. compiler .. " >/dev/null 2>&1")
end

local function find_compiler()
	local c_compilers = { "clang", "cc", "gcc", "tcc" }
	local CC = os.getenv("CC")
	if CC ~= nil then
		if compiler_exists(CC) then
			return CC
		end
		io.stderr:write("[WARN]: CC is not set to an existing compiler (check your $PATH). Using a fallback.\n")
	end

	for _, c in ipairs(c_compilers) do
		if compiler_exists(c) then
			return c
		end
	end

	-- Like, what do we even do now? This is a total disaster!
	io.stderr:write("[FATAL]: Could not find a C compiler! Install one, bestie!\n")
	os.exit(1)
end

return {
	-- compiler = "cc",
	compiler = find_compiler(),
	name = "pb",
	-- This will be prefix'd with -D
	defines = {
		"DEBUG",
	},
	-- CLFAGS will be apppended as is for now
	cflags = {
		"-std=c23",
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
