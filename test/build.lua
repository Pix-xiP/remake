local function foo()
	return "value"
end

return {
	text = "Some target",
	number = 1,
	func = foo(),
	bool = true,
	float = 2.0,

	tabler = { nested = "foo" }, -- TODO: Parse nested tables.
	nest = {
		big_nest = true,
		nest_two = {
			bar = "barstring",
			final = {
				final_baz = 1.0,
			},
			-- 	"foo",
			-- 	"bar",
			-- },
		},
	},

	-- { field = "foobar" }, -- TODO: Parse anonymous tables.
}
