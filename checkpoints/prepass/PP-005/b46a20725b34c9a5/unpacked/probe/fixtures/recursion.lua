local function recur(n) return 1 + recur(n+1) end
return recur(0)
