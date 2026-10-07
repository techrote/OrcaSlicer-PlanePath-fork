local retained = {}
for i = 1, 1000000 do retained[i] = {i, i+1, i+2, i+3} end
return #retained
