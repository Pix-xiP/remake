-- do more function stuff here perhaps?
-- identify the compiler even better?
-- locate dependencies too?

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

	-- Like, what do we even do now?
	io.stderr:write("[FATAL]: Could not find a C compiler!\n")
	os.exit(1)
end

return {
	build_dir = "./build",
	install = {
		directory = "./bin",
	},
	targets = {
		{
			name = "pb",
			kind = "exe",
			compiler = find_compiler(),
			defines = {
				"DEBUG",
			},
			cflags = {
				"-std=c23",
				"-ggdb",
				"-fPIC",
				"-fsanitize=address",
			},
			sources = {
				"src/pb_file_checker.c",
				"src/pb_main.c",
				"src/pb_parsing.c",
			},
			include_dirs = {
				"./src",
				"./lua-5.4.6/install/include",
				"./mimalloc/include",
			},
			libs = {
				"lua",
				"m",
			},
			library_dirs = {
				"./lua-5.4.6/install/lib",
			},
			optimisation_level = "default",
		},
	},
}
