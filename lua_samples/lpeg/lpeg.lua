-- Lpeg test
-- match string
-- For more details of lpeg, visit the lpeg homepage show bellow:
-- http://www.inf.pub-rio.br/~roberto/lpeg/

match = lpeg.match;
P = lpeg.P;

res = match(P'a', 'aaa');	-- return the index behind first match 'a'
print(res);

res = match(P'a', '123');
print(res);		-- output nil, since match nothing	
