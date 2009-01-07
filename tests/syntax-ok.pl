foo(X, Y) :- bar(X), foobar(X, Y).

bar(X) :- foobar(X, X).


bar(a).
bar(b).
bar(c).
foobar(a, a).
foobar(a, b).
foobar(a, c).
foobar(b, c).
foobar(c, a).
foobar(c, b).
foobar(c, c).
