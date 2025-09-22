local lacpi = require("lacpi")

local function test_walk_namespace()
	local nodes = lacpi.walk.dump_namespace()  -- returns table of tables

	-- Print them in Lua
	for _, node in ipairs(nodes) do
	    print(node.level, node.path, node.HID or "", node.UID or "")
	end

	return "SUCCESS";
end

print("--- TEST BEGIN: WALK NS ---");

status = test_walk_namespace()

print("--- TEST END: WALK NS ---")
print(status)
