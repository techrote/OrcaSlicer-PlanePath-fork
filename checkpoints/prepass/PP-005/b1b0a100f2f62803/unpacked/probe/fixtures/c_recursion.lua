local function f() host_reenter(f) end
-- recursive local binding must exist before its body refers to f
f()
