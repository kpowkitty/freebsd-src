local function test_walk_namespace()
	local nodes = lacpi.walk.namespace()  -- returns table of tables

	-- Print them in Lua
	for _, node in ipairs(nodes) do
	    print(node.level, node.path, node.HID, node.UID)
	end

	return "SUCCESS"
end

print("--- TEST BEGIN: WALK NS ---")

status = test_walk_namespace()

print("--- TEST END: WALK NS ---")
print(status)
