local function check_compiler(compiler)
    local _, _, signal = os.execute(compiler .. "&>/dev/null")
    return signal == 1
end

local c_compilers = { "cc", "gcc", "clang", "tcc" }
-- do more function stuff here perhaps?
-- identify the compiler even better?
-- locate dependencies too?
local function get_compiler()
    local CC = os.getenv("CC")
    if CC ~= nil then
        if check_compiler(CC) then
            return CC
        end
        io.stderr:write("WARNING: CC is not set to an existing compiler (check your $PATH). Using a fallback.\n")
    end

    for _, c in ipairs(c_compilers) do
        check_compiler(c)
    end
end


return {
	-- compiler = "cc",
	compiler = fetch_compiler,
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
